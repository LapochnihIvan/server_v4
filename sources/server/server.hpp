#ifndef SERVER_HPP
#define SERVER_HPP

#include <functional>
#include <set>

#include "database/database_query.hpp"

#include "get_request.hpp"
#include "request_handler.hpp"

#undef CROW_STATIC_DIRECTORY
#define CROW_STATIC_DIRECTORY "assets/"

#undef CROW_STATIC_ENDPOINT
#define CROW_STATIC_ENDPOINT "/assets/<path>"

#include "core/journal_handler.hpp"

#include "crow.h"

//--------------------------------------------------------------------------------
namespace serv
{
class Server //: public RequestHandler<Server>
{
public:
    Server(data::DBSettings aDBS);

    std::string uploadFile(crow::multipart::message& msg);

    //--------------------------------------------------------------------------------

    template <typename T>
    crow::json::wvalue::list getData(
        GetRequest aRequest, const std::string& aCondition = "") noexcept
    {

        crow::json::wvalue::list res;

        if (T::tableName == "role" && aCondition.size() > 0)
        {
            data::DatabaseQuery dbq(mDBS);
            auto table  = dbq.getData<data::Role>();
            int nameNum = table.names["name"];
            for (auto& i : table)
            {
                roles.emplace_back(*(std::string*)i[nameNum]);
            }

            auto it = aCondition.end();
            while (*(--it) != ' ')
                ;
            std::string temp(it, aCondition.end());
            // std::cout << temp << "\n";
            int rol = std::stoi(temp);

            for (int i = 0; rol; ++i)
            {
                if (rol & 1)
                {
                    res.emplace_back(roles[i]);
                }

                rol >>= 1;
            }
        }
        else
        {
            data::DatabaseQuery dbq(mDBS);
            auto table = dbq.getData<T>(aCondition);

            for (auto& i : table)
            {
                crow::json::wvalue temp;
                // for (auto& j : table.names)
                for (auto& j : aRequest)
                {
                    std::string name(j.first);
                    if (name == "*") continue;
                    // size_t indx      = j.second;

                    // auto opt         = aRequest[name];
                    // if (!opt.has_value()) continue;

                    auto it    = table.names.find(name);
                    bool notID = false;
                    if (it == table.names.end())
                    {
                        it    = table.names.find("id");
                        notID = true;
                    }
                    size_t indx = it->second;

                    // if (name == "role_id" || opt.get().second.size() > 1)
                    if (name == "role_id")
                    {
                        temp["roles"] =
                            getDataAsJSON("role", GetRequest(),
                                          "id = " + data::wrap(*(int*)i[indx]));
                    }
                    else if (j.second.size() > 0)
                    {
                        std::string cond = "id = ";
                        if (!notID) name.resize(name.size() - 3);
                        else
                        {
                            cond = aRequest.name;
                            cond += "_id = "s;
                        }
                        auto req = GetRequest(j.second, name);

                        auto jList = getDataAsJSON(
                            req.name, req, cond + data::wrap(*(int*)i[indx]));
                        if (jList.size() != 1)
                        {
                            temp[name + "s"] = std::move(jList);
                        }
                        else
                        {
                            temp[name] = std::move(jList[0]);
                        }
                    }
                    else
                    {
                        switch (table.types[indx])
                        {
                            case data::Type::INT:
                                temp[name] = *(int*)i[indx];
                                break;
                            case data::Type::BOOL:
                                temp[name] = *(bool*)i[indx];
                                break;
                            case data::Type::CHARS:
                                temp[name] = std::string((char*)i[indx]);
                                break;
                            case data::Type::STRING:
                                temp[name] = *(std::string*)i[indx];
                                break;
                        }
                    }
                }
                res.emplace_back(std::move(temp));
            }
        }
        return res;
    }

    //--------------------------------------------------------------------------------

    crow::response get(const std::string& aRequest,
                       std::string&& aCondition) noexcept
    {
        for (auto& i : aCondition)
        {
            if (i == ';') i = ' ';
        }
        GetRequest req(aRequest);
        if (req.args.size() == 0)
        {
            req.args["*"];
        }
        req.reset(req.name);
        auto data = getDataAsJSON(req.name, req, aCondition);

        crow::response res;
        if (data.size() == 0)
        {
            res = std::move(crow::response{401});
        }
        else
        {
            crow::json::wvalue temp;
            std::string tempName(req.name);
            if (data.size() > 1)
            {
                tempName += "s";
                temp[tempName] = std::move(data);
            }
            else
            {
                temp[tempName] = std::move(data[0]);
            }

            // crow::json::wvalue
            //     temp[data.size() > 1 ? "s" + std::string(req.name) :
            //     req.name] =
            //         data;
            res = std::move(temp);
        }

        return res;
    }

    //--------------------------------------------------------------------------------

private:
    data::DBSettings mDBS;

    std::vector<std::string> roles;

    SERVER_FUNCTIONS

};
} // namespace serv
//--------------------------------------------------------------------------------

#endif // !SERVER_HPP
