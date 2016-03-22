#include "httphandler.h"
#include "httprequest.h"
#include "httpresponse.h"

#include "kkm_protocol.h"

#include <qjson/parser.h>

HttpHandler::HttpHandler(KKM_Protocol* prot) : HttpRequestHandler()
{
    _proto = prot;
}

void HttpHandler::service(HttpRequest &request, HttpResponse &response)
{
    if(request.getMethod() == "GET") {

        if( request.getPath() == "/"){
            response.write("Druff ATOL driver is on");
            return;
        }

        if(request.getPath() == "/api/beep/"){

            if(_proto->thr_beep())
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        response.setStatus(404);
        return;
    }

    if(request.getMethod() == "POST"){

        if(request.getPath() == "/api/begin_session/"){
            if(_proto->thr_sessionOpen())
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/end_session/"){
            if(_proto->thr_sessionClose())
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/open_check/"){
            if(_proto->thr_checkOpen())
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/close_check/"){

            QJson::Parser   pars;
            bool            isOk = false;
            QVariantMap     res = pars.parse(request.getBody(), &isOk).toMap();

            if(isOk == false)
                response.setStatus(400);

            float   sum = res.value("sum").toFloat();
            float   discount = res.value("discount").toFloat();

            if(_proto->thr_checkClose(sum, discount))
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/cancel_check/"){

            if(_proto->thr_checkCancel())
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/add_check_entry/"){

            QJson::Parser   pars;
            bool            isOk = false;
            QVariantMap     res = pars.parse(request.getBody(), &isOk).toMap();

            if(isOk == false)
                response.setStatus(400);

            QString     name = res.value("name").toString();
            if(name.isEmpty() == false)
                response.setStatus(400);

            float       price = res.value("price").toFloat();
            float       count = res.value("count").toFloat();
            char        section = res.value("section").toInt();

            if(_proto->thr_checkAddEntry(name, price, count, section))
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/beep/"){

            if(_proto->thr_beep())
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/print_x/"){

            if(_proto->thr_printX())
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/print/"){

            QJson::Parser   pars;
            bool            isOk = false;
            QVariantMap     res = pars.parse(request.getBody(), &isOk).toMap();

            if(isOk == false)
                response.setStatus(400);

            QString     text = res.value("text").toString();

            if(_proto->thr_print(text))
                response.setStatus(201);
            else
                response.setStatus(400);
            return;
        }

        if(request.getPath() == "/api/print_check/"){

            QJson::Parser   pars;
            bool            isOk = false;
            QVariantMap     res = pars.parse(request.getBody(), &isOk).toMap();
            if(isOk == false)
                response.setStatus(400);

            QVariantList    lst = res.value("entry_list").toList();

            if(lst.count() == 0)
                response.setStatus(400);

            QList<CheckEntry>   chk;
            CheckEntry          ent;
            foreach(const QVariant &var, lst){
                QVariantMap map = var.toMap();

                ent.desc = map.value("name").toString();
                if(ent.desc.isEmpty() == false)
                    response.setStatus(400);

                ent.price = map.value("price").toFloat();
                ent.count = map.value("count").toFloat();
                ent.section = map.value("section").toInt();
                chk.append(ent);
            }

            float   sum = res.value("sum").toFloat();
            float   discount = res.value("discount").toFloat();

            if(_proto->checkPrint(chk, sum, discount))
                response.setStatus(201);
            else
                response.setStatus(400);

            return;
        }
    }

    response.setStatus(404);
}

