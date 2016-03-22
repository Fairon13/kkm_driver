#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include "httprequesthandler.h"

class KKM_Protocol;
class HttpHandler : public HttpRequestHandler
{
    Q_OBJECT

    KKM_Protocol*   _proto;

public:
    HttpHandler(KKM_Protocol* prot);

    void service(HttpRequest& request, HttpResponse& response);
};

#endif // HTTPHANDLER_H
