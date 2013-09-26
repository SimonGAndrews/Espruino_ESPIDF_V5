/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains HTTP client and server
 * ----------------------------------------------------------------------------
 */
#include "httpserver.h"
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#include "jsparse.h"
#include "jsinteractive.h"
#ifdef USE_CC3000
#else
#include <sys/stat.h>
#include <errno.h>
#endif


// -----------------------------

#define HTTP_ON_CONNECT "#onconnect"
#define HTTP_ON_DATA "#ondata"

#define LIST_ADD(list, item)  \
  item->prev = 0;              \
  item->next = list;           \
  if (list) list->prev = item; \
  list = item;

#define LIST_REMOVE(list, item)                  \
  if (list==item) list=item->next;               \
  if (item->prev) item->prev->next = item->next;  \
  if (item->next) item->next->prev = item->prev;  \
  item->prev = 0;                                 \
  item->next = 0;


// -----------------------------

HttpServer *httpServers;
HttpServerConnection *httpServerConnections;
HttpClientConnection *httpClientConnections;

// -----------------------------
void httpError(const char *msg) {
  //WSAGetLastError?
  jsError(msg);
}

void httpAppendHeaders(JsVar *string, JsVar *headerObject) {
  // append headers
  JsObjectIterator it;
  jsvObjectIteratorNew(&it, headerObject);
  while (jsvObjectIteratorHasElement(&it)) {
    JsVar *k = jsvAsString(jsvObjectIteratorGetKey(&it), true);
    JsVar *v = jsvAsString(jsvObjectIteratorGetValue(&it), true);
    jsvAppendStringVarComplete(string, k);
    jsvAppendString(string, ": ");
    jsvAppendStringVarComplete(string, v);
    jsvAppendString(string, "\r\n");
    jsvUnLock(k);
    jsvUnLock(v);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  // free headers
}
// -----------------------------

void httpServerInit() {
  httpServers = 0;
  httpServerConnections = 0;
  httpClientConnections = 0;
#ifdef WIN32
  // Init winsock 1.1
  WORD sockVersion;
  WSADATA wsaData;
  sockVersion = MAKEWORD(1, 1);
  WSAStartup(sockVersion, &wsaData);
#endif
}

void _httpServerConnectionKill(HttpServerConnection *connection) {
  if (connection->socket!=-1) close(connection->socket);
  jsvUnLock(connection->var);
  jsvUnLock(connection->resVar);
  jsvUnLock(connection->reqVar);
  jsvUnLock(connection->sendHeaders);
  jsvUnLock(connection->sendData);
  jsvUnLock(connection->receiveData);
  free(connection);
}

void _httpClientConnectionKill(HttpClientConnection *connection) {
  if (connection->socket!=-1) close(connection->socket);
  jsvUnLock(connection->resVar);
  jsvUnLock(connection->reqVar);
  jsvUnLock(connection->sendData);
  jsvUnLock(connection->receiveData);
  jsvUnLock(connection->options);
  free(connection);
}

void httpServerKill() {
  // shut down connections
  {
    HttpServerConnection *connection = httpServerConnections;
    httpServerConnections = 0;
    while (connection) {
      HttpServerConnection *oldConnection = connection;
      connection = connection->next;
      _httpServerConnectionKill(oldConnection);
    }
  }
  {
    HttpClientConnection *connection = httpClientConnections;
    httpClientConnections = 0;
    while (connection) {
      HttpClientConnection *oldConnection = connection;
      connection = connection->next;
      _httpClientConnectionKill(oldConnection);
    }
  }
  // shut down our listeners, unlock objects, free data
  HttpServer *server = httpServers;
  while (server) {
    jsvUnLock(server->var);
    close(server->listeningSocket);
    HttpServer *oldServer = server;
    server = server->next;
    free(oldServer);
  }
  httpServers=0;

#ifdef WIN32
   // Shutdown Winsock
   WSACleanup();
#endif
}

// httpParseHeaders(&connection->receiveData, connection->reqVar, true) // server
// httpParseHeaders(&connection->receiveData, connection->resVar, false) // client

bool httpParseHeaders(JsVar **receiveData, JsVar *objectForData, bool isServer) {
  // find /r/n/r/n
  int newlineIdx = 0;
  int strIdx = 0;
  int headerEnd = -1;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, *receiveData, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch == '\r') {
      if (newlineIdx==0) newlineIdx=1;
      else if (newlineIdx==2) newlineIdx=3;
    } else if (ch == '\n') {
      if (newlineIdx==1) newlineIdx=2;
      else if (newlineIdx==3) {
        headerEnd = strIdx+1;
      }
    } else newlineIdx=0;
    jsvStringIteratorNext(&it);
    strIdx++;
  }
  jsvStringIteratorFree(&it);
  // skip if we have no header
  if (headerEnd<0) return false;
  // Now parse the header
  JsVar *vHeaders = jsvNewWithFlags(JSV_OBJECT);
  if (!vHeaders) return true;
  jsvUnLock(jsvAddNamedChild(objectForData, vHeaders, "headers"));
  strIdx = 0;
  int firstSpace = -1;
  int secondSpace = -1;
  int lineNumber = 0;
  int lastLineStart = 0;
  int colonPos = 0;
  //jsiConsolePrintStringVar(receiveData);
  jsvStringIteratorNew(&it, *receiveData, 0);
    while (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetChar(&it);
      if (ch==' ' || ch=='\r') {
        if (firstSpace<0) firstSpace = strIdx;
        else if (secondSpace<0) secondSpace = strIdx;
      }
      if (ch == ':' && colonPos<0) colonPos = strIdx;
      if (ch == '\r') {
        if (lineNumber>0 && colonPos>lastLineStart && lastLineStart<strIdx) {
          JsVar *hVal = jsvNewFromEmptyString();
          if (hVal)
            jsvAppendStringVar(hVal, *receiveData, colonPos+2, strIdx-(colonPos+2));
          JsVar *hKey = jsvNewFromEmptyString();
          if (hKey) {
            jsvMakeIntoVariableName(hKey, hVal);
            jsvAppendStringVar(hKey, *receiveData, lastLineStart, colonPos-lastLineStart);
            jsvAddName(vHeaders, hKey);
            jsvUnLock(hKey);
          }
          jsvUnLock(hVal);
        }
        lineNumber++;
        colonPos=-1;
      }
      if (ch == '\r' || ch == '\n') {
        lastLineStart = strIdx+1;
      }

      jsvStringIteratorNext(&it);
      strIdx++;
    }
    jsvStringIteratorFree(&it);
  jsvUnLock(vHeaders);
  // try and pull out methods/etc
  if (isServer) {
    JsVar *vMethod = jsvNewFromEmptyString();
    if (vMethod) {
      jsvAppendStringVar(vMethod, *receiveData, 0, firstSpace);
      jsvUnLock(jsvAddNamedChild(objectForData, vMethod, "method"));
      jsvUnLock(vMethod);
    }
    JsVar *vUrl = jsvNewFromEmptyString();
    if (vUrl) {
      jsvAppendStringVar(vUrl, *receiveData, firstSpace+1, secondSpace-(firstSpace+1));
      jsvUnLock(jsvAddNamedChild(objectForData, vUrl, "url"));
      jsvUnLock(vUrl);
    }
  }
  // strip out the header
  JsVar *afterHeaders = jsvNewFromEmptyString();
  if (!afterHeaders) return true;
  jsvAppendStringVar(afterHeaders, *receiveData, headerEnd, JSVAPPENDSTRINGVAR_MAXLENGTH);
  jsvUnLock(*receiveData);
  *receiveData = afterHeaders;
  return true;
}

void httpServerConnectionsIdle() {
  HttpServerConnection *connection = httpServerConnections;
  while (connection) {
    HttpServerConnection *nextConnection = connection->next;
    // TODO: look for unreffed connections?
    fd_set s;
    FD_ZERO(&s);
    FD_SET(connection->socket,&s);
    // check for waiting clients
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    int n = select(connection->socket+1,&s,NULL,NULL,&timeout);
    if (n==SOCKET_ERROR) {
      // we probably disconnected so just get rid of this
      connection->closeNow = true;
    } else if (n>0) {
       char buf[256];
       int num = 256;
       while (num==256) {
          // receive data
          num = (int)recv(connection->socket,buf,256,0);
          // add it to our request string
          if (num>0) {
            if (!connection->receiveData) connection->receiveData = jsvNewFromEmptyString();
            if (connection->receiveData) {
              jsvAppendStringBuf(connection->receiveData, buf, num);
              if (!connection->hadHeaders && httpParseHeaders(&connection->receiveData, connection->reqVar, true)) {
                connection->hadHeaders = true;
                jsiQueueObjectCallbacks(connection->var, HTTP_ON_CONNECT, connection->reqVar, connection->resVar);
              }
            }
          }
       }
    }

    // send data if possible
    if (connection->sendData) {
       int a=1;
       int len = (int)jsvGetStringLength(connection->sendData);
       if (len>0) {
         char *data = (char*)malloc((size_t)(len+1));
         jsvGetString(connection->sendData, data, (size_t)(len+1));
         a = (int)send(connection->socket,data,(size_t)len, MSG_NOSIGNAL);
         jsvUnLock(connection->sendData); connection->sendData = 0;
         if (a!=len) {
           connection->sendData = jsvNewFromEmptyString();
           jsvAppendStringBuf(connection->sendData, &data[a], len-a);
         }
         free(data);
       }
       if (a<=0) {
         httpError("Socket error while sending");
         connection->closeNow = true;
       }
    }
    if (connection->close && !connection->sendData)
      connection->closeNow = true;

    if (connection->closeNow) {
      LIST_REMOVE(httpServerConnections, connection);
      _httpServerConnectionKill(connection);
    }
    connection = nextConnection;
  }
}

void httpClientConnectionsIdle() {
  HttpClientConnection *connection = httpClientConnections;
  while (connection) {
    HttpClientConnection *nextConnection = connection->next;

    /* We do this up here because we want to wait until we have been once
     * around the idle loop (=callbacks have been executed) before we run this */
    if (connection->hadHeaders && connection->receiveData) {
      jsiQueueObjectCallbacks(connection->resVar, HTTP_ON_DATA, connection->receiveData, 0);
      // clear - because we have issued a callback
      jsvUnLock(connection->receiveData);
      connection->receiveData = 0;
    }

    if (connection->socket!=-1) {
      // send data if possible
      if (connection->sendData) {
        // this will wait to see if we can write any more, but ALSO
        // will wait for connection
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(connection->socket, &writefds);
        struct timeval time;
        time.tv_sec = 0;
        time.tv_usec = 0;
        int n = select(connection->socket+1/* ? */, 0, &writefds, 0, &time);
        if (n==SOCKET_ERROR ) {
           // we probably disconnected so just get rid of this
          connection->closeNow = true;
          return;
        }
        if (FD_ISSET(connection->socket, &writefds)) {
          int a=1;
          int len = (int)jsvGetStringLength(connection->sendData);
          if (len>0) {
            char *data = (char*)malloc((size_t)(len+1));
            jsvGetString(connection->sendData, data, (size_t)(len+1));
            a = (int)send(connection->socket,data,(size_t)len, MSG_NOSIGNAL);
            jsvUnLock(connection->sendData); connection->sendData = 0;
            if (a!=len) {
              connection->sendData = jsvNewFromEmptyString();
              jsvAppendStringBuf(connection->sendData, &data[a], len-a);
            }
            free(data);
          }
          if (a<=0) {
            httpError("Socket error while sending");
            connection->closeNow = true;
          }
        }
      }

      // Now receive data
      fd_set s;
      FD_ZERO(&s);
      FD_SET(connection->socket,&s);
      // check for waiting clients
      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      int n = select(connection->socket+1,&s,NULL,NULL,&timeout);
      if (n==SOCKET_ERROR) {
        // we probably disconnected so just get rid of this
        connection->closeNow = true;
      } else if (n>0) {
         char buf[256];
         int num = 256;
         while (num==256) {
            // receive data
            num = (int)recv(connection->socket,buf,256,0);
            // add it to our request string
            if (num>0) {
              if (!connection->receiveData)
                connection->receiveData = jsvNewFromEmptyString();
              if (connection->receiveData) {
                jsvAppendStringBuf(connection->receiveData, buf, num);
                if (!connection->hadHeaders) {
                  if (httpParseHeaders(&connection->receiveData, connection->resVar, false)) {
                    connection->hadHeaders = true;
                    jsiQueueObjectCallbacks(connection->reqVar, HTTP_ON_CONNECT, connection->resVar, 0);
                  }
                }
              }
            }
         }
      }
    }

    if (connection->closeNow) {
      LIST_REMOVE(httpClientConnections, connection);
      _httpClientConnectionKill(connection);
    }
    connection = nextConnection;
  }
}


void httpServerIdle() {
  HttpServer *server = httpServers;
  while (server) {
    // TODO: look for unreffed servers?
    fd_set s;
    FD_ZERO(&s);
    FD_SET(server->listeningSocket,&s);
    // check for waiting clients
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    int n = select(server->listeningSocket+1,&s,NULL,NULL,&timeout);

    while (n-->0) {
      // we have a client waiting to connect...
      SOCKET theClient; // connect it
      theClient = accept(server->listeningSocket,NULL,NULL);
       if (theClient != INVALID_SOCKET) {
         JsVar *req = jspNewObject(jsiGetParser(), 0, "httpSRq");
         JsVar *res = jspNewObject(jsiGetParser(), 0, "httpSRs");
         if (res && req) { // out of memory?
           HttpServerConnection *connection = (HttpServerConnection*)malloc(sizeof(HttpServerConnection));
           connection->var = jsvLockAgain(server->var);
           connection->reqVar = jsvLockAgain(req);
           connection->resVar = jsvLockAgain(res);
           connection->socket = theClient;
           connection->sendCode = 200;
           connection->sendHeaders = jsvNewWithFlags(JSV_OBJECT);
           connection->sendData = 0;
           connection->receiveData = 0;
           connection->close = false;
           connection->closeNow = false;
           connection->hadHeaders = false;
           LIST_ADD(httpServerConnections, connection);
         }
         jsvUnLock(req);
         jsvUnLock(res);
         //add(new CNetworkConnect(theClient, this));
         // add to service queue
      }
    }

    server = server->next;
  }
  httpServerConnectionsIdle();
  httpClientConnectionsIdle();
}

// -----------------------------

HttpServer *httpFindServer(JsVar *httpServerVar) {
  HttpServer *server = httpServers;
  while (server) {
    if (server->var == httpServerVar) return server;
    server = server->next;
  }
  return 0;
}

HttpServerConnection *httpFindServerConnectionFromResponse(JsVar *httpServerResponseVar) {
  HttpServerConnection *connection = httpServerConnections;
  while (connection) {
    if (connection->resVar == httpServerResponseVar) return connection;
    connection = connection->next;
  }
  return 0;
}

HttpClientConnection *httpFindHttpClientConnectionFromRequest(JsVar *httpClientRequestVar) {
  HttpClientConnection *connection = httpClientConnections;
    while (connection) {
      if (connection->reqVar == httpClientRequestVar) return connection;
      connection = connection->next;
    }
    return 0;
}


// -----------------------------

JsVar *httpServerNew(JsVar *callback) {
  JsVar *serverVar = jspNewObject(jsiGetParser(),0,"httpSrv");
  if (!serverVar) return 0; // out of memory
  jsvUnLock(jsvAddNamedChild(serverVar, callback, HTTP_ON_CONNECT));

  HttpServer *server = (HttpServer*)malloc(sizeof(HttpServer));
  server->var = jsvLockAgain(serverVar);
  LIST_ADD(httpServers, server);

  server->listeningSocket = socket(AF_INET,           // Go over TCP/IP
                                   SOCK_STREAM,       // This is a stream-oriented socket
                                   IPPROTO_TCP);      // Use TCP rather than UDP
  if (server->listeningSocket == INVALID_SOCKET) {
    httpError("httpServer: socket");
    return serverVar;
  }

  int optval = 1;
  if (setsockopt(server->listeningSocket,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(optval))==-1)
    jsWarn("http: setsockopt failed\n");

  return serverVar;
}

void httpServerListen(JsVar *httpServerVar, int port) {
  HttpServer *server = httpFindServer(httpServerVar);
  if (!server) return;

  int nret;
  sockaddr_in serverInfo;
  serverInfo.sin_family = AF_INET;
  if (false)
    serverInfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // allow only LOCAL clients to connect
  else
    serverInfo.sin_addr.s_addr = INADDR_ANY; // allow anyone to connect

  serverInfo.sin_port = htons((unsigned short)port); // port
  nret = bind(server->listeningSocket, (struct sockaddr*)&serverInfo, sizeof(struct sockaddr));
  if (nret == SOCKET_ERROR) {
    httpError("httpServer: bind");
    close(server->listeningSocket);
    return;
  }

  // Make the socket listen
  nret = listen(server->listeningSocket, 10); // 10 connections
  if (nret == SOCKET_ERROR) {
    httpError("httpServer: listen");
    close(server->listeningSocket);
    return;
  }
}


JsVar *httpClientRequestNew(JsVar *options, JsVar *callback) {
  JsVar *req = jspNewObject(jsiGetParser(), 0, "httpCRq");
  JsVar *res = jspNewObject(jsiGetParser(), 0, "httpCRs");
  if (res && req) { // out of memory?
   jsvUnLock(jsvAddNamedChild(req, callback, HTTP_ON_CONNECT));

   HttpClientConnection *connection = (HttpClientConnection*)malloc(sizeof(HttpClientConnection));
   connection->reqVar = jsvLockAgain(req);
   connection->resVar = jsvLockAgain(res);
   connection->socket = -1;
   connection->sendData = 0;
   connection->receiveData = 0;
   connection->closeNow = false;
   connection->hadHeaders = false;
   connection->options = jsvLockAgain(options);
   LIST_ADD(httpClientConnections, connection);
  }
  jsvUnLock(res);
  return req;
}

void httpClientRequestWrite(JsVar *httpClientReqVar, JsVar *data) {
  HttpClientConnection *connection = httpFindHttpClientConnectionFromRequest(httpClientReqVar);
  if (!connection) return;
  // Append data to sendData
  if (!connection->sendData) {
    if (connection->options) {
      connection->sendData = jsvNewFromString("");
      JsVar *method = jsvSkipNameAndUnLock(jsvFindChildFromString(connection->options, "method", false));
      jsvAppendStringVarComplete(connection->sendData, method);
      jsvUnLock(method);
      jsvAppendString(connection->sendData, " ");
      JsVar *path = jsvSkipNameAndUnLock(jsvFindChildFromString(connection->options, "path", false));
      jsvAppendStringVarComplete(connection->sendData, path);
      jsvUnLock(path);
      jsvAppendString(connection->sendData, " HTTP/1.0\r\nUser-Agent: Espruino "JS_VERSION"\r\nConnection: close\r\n");
      JsVar *headers = jsvSkipNameAndUnLock(jsvFindChildFromString(connection->options, "headers", false));
      bool hasHostHeader = false;
      if (jsvIsObject(headers)) {
        JsVar *hostHeader = jsvSkipNameAndUnLock(jsvFindChildFromString(headers, "Host", false));
        hasHostHeader = hostHeader!=0;
        jsvUnLock(hostHeader);
        httpAppendHeaders(connection->sendData, headers);
      }
      jsvUnLock(headers);
      if (!hasHostHeader) {
        JsVar *host = jsvSkipNameAndUnLock(jsvFindChildFromString(connection->options, "host", false));
        JsVarInt port = jsvGetIntegerAndUnLock(jsvSkipNameAndUnLock(jsvFindChildFromString(connection->options, "port", false)));
        jsvAppendString(connection->sendData, "Host: ");
        jsvAppendStringVarComplete(connection->sendData, host);
        if (port>0 && port!=80) {
          jsvAppendString(connection->sendData, ":");
          jsvAppendInteger(connection->sendData, port);
        }
        jsvAppendString(connection->sendData, "\r\n");
        jsvUnLock(host);
      }
      // finally add ending newline
      jsvAppendString(connection->sendData, "\r\n");
    } else {
      connection->sendData = jsvNewFromString("");
    }
  }
  if (data && connection->sendData) {
    JsVar *s = jsvAsString(data, false);
    if (s) jsvAppendStringVarComplete(connection->sendData,s);
    jsvUnLock(s);
  }
}

void httpClientRequestEnd(JsVar *httpClientReqVar) {
  httpClientRequestWrite(httpClientReqVar, 0); // force sendData to be made

  HttpClientConnection *connection = httpFindHttpClientConnectionFromRequest(httpClientReqVar);
  if (!connection) return;

  connection->socket = socket (AF_INET, SOCK_STREAM, 0);
  if (connection->socket == -1) {
     httpError("Unable to create socket\n");
     connection->closeNow = true;
  }

  sockaddr_in       sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons( (unsigned short)jsvGetIntegerAndUnLock(jsvSkipNameAndUnLock(jsvFindChildFromString(connection->options, "port", false))) );


  char hostName[128];
  JsVar *hostNameVar = jsvSkipNameAndUnLock(jsvFindChildFromString(connection->options, "host", false));
  jsvGetString(hostNameVar, hostName, sizeof(hostName));
  jsvUnLock(hostNameVar);
  struct hostent * host_addr = gethostbyname(hostName);
  /* getaddrinfo is newer than this?
  *
  * struct addrinfo * result;
  * error = getaddrinfo("www.example.com", NULL, NULL, &result);
  * if (0 != error)
  *   fprintf(stderr, "error %s\n", gai_strerror(error));
  *
  */
  if(host_addr==NULL) {
    httpError("Unable to locate host");
    connection->closeNow = true;
    return;
  }

  // turn on non-blocking mode
  #ifdef WIN_OS
  u_long n = 1;
  ioctlsocket(connection->socket,FIONBIO,&n);
  #else
  int flags = fcntl(connection->socket, F_GETFL);
  if (flags < 0) {
     httpError("Unable to retrieve socket descriptor status flags");
     connection->closeNow = true;
     return;
  }
  if (fcntl(connection->socket, F_SETFL, flags | O_NONBLOCK) < 0)
     httpError("Unable to set socket descriptor status flags\n");
  #endif

  sin.sin_addr.s_addr = (in_addr_t)*((int*)*host_addr->h_addr_list) ;
  //uint32_t a = sin.sin_addr.s_addr;
  //_DEBUG_PRINT( cout<<"Port :"<<sin.sin_port<<", Address : "<< sin.sin_addr.s_addr<<endl);
  int res = connect (connection->socket,(const struct sockaddr *)&sin, sizeof(sockaddr_in) );
  if (res == SOCKET_ERROR) {
  #ifdef WIN_OS
   int err = WSAGetLastError();
  #else
   int err = errno;
  #endif
   if (err != EINPROGRESS &&
       err != EWOULDBLOCK) {
     httpError("Connect failed\n" );
     connection->closeNow = true;
   }
  }
}


void httpServerResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *headers) {
  HttpServerConnection *connection = httpFindServerConnectionFromResponse(httpServerResponseVar);
  if (!connection) return;
  if (!jsvIsUndefined(headers) && !jsvIsObject(headers)) {
    httpError("Headers sent to writeHead should be an object");
    return;
  }

  connection->sendCode = statusCode;
  if (connection->sendHeaders) {
    if (!jsvIsUndefined(headers)) {
      jsvUnLock(connection->sendHeaders);
      connection->sendHeaders = jsvLockAgain(headers);
    }
  } else {
    httpError("Headers have already been sent");
  }
}


void httpServerResponseData(JsVar *httpServerResponseVar, JsVar *data) {
  HttpServerConnection *connection = httpFindServerConnectionFromResponse(httpServerResponseVar);
  if (!connection) return;
  // Append data to sendData
  if (!connection->sendData) {
    if (connection->sendHeaders) {
      connection->sendData = jsvNewFromString("HTTP/1.0 ");
      jsvAppendInteger(connection->sendData, connection->sendCode);
      jsvAppendString(connection->sendData, " OK\r\nServer: Espruino "JS_VERSION"\r\n");
      httpAppendHeaders(connection->sendData, connection->sendHeaders);
      jsvUnLock(connection->sendHeaders);
      connection->sendHeaders = 0;
      // finally add ending newline
      jsvAppendString(connection->sendData, "\r\n");
    } else {
      // we have already sent headers
      connection->sendData = jsvNewFromEmptyString();
    }
  }
  if (connection->sendData && !jsvIsUndefined(data)) {
    JsVar *s = jsvAsString(data, false);
    if (s) jsvAppendStringVarComplete(connection->sendData,s);
    jsvUnLock(s);
  }
}

void httpServerResponseEnd(JsVar *httpServerResponseVar) {
  httpServerResponseData(httpServerResponseVar, 0); // force onnection->sendData to be created even if data not called
  HttpServerConnection *connection = httpFindServerConnectionFromResponse(httpServerResponseVar);
  if (!connection) return;
  connection->close = true;
}
