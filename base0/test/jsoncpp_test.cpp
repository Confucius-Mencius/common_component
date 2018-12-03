#include "jsoncpp_test.h"
#include "json/json.h"

static int ReadJson(const std::string& str)
{
    try
    {
        Json::Reader reader;
        Json::Value value;

        if (reader.parse(str, value))
        {
            // 解析json中的对象
            std::cout << "name : " << value["name"].asString() << std::endl;
            std::cout << "number : " << value["number"].asInt() << std::endl;
            std::cout << "value : " << value["value"].asBool() << std::endl;
            std::cout << "no such num : " << value["haha"].asInt() << std::endl;
            std::cout << "no such str : " << value["hehe"].asString() << std::endl;

            //解析数组对象
            const Json::Value num_array = value["num_array"];

            for (int i = 0; i < (int) num_array.size(); ++i)
            {
                std::cout << "num_array[" << i << "] = " << num_array[i] << std::endl;
            }

            // 解析对象数组对象
            Json::Value obj_array = value["obj_array"];
            std::cout << "obj array_ size_ = " << obj_array.size() << std::endl;

            for (int i = 0; i < (int) obj_array.size(); ++i)
            {
                std::cout << obj_array[i];
            }

            // 从对象数组中找到想要的对象
            for (int i = 0; i < (int) obj_array.size(); i++)
            {
                if (obj_array[i].isMember("string"))
                {
                    std::cout << "string : " << obj_array[i]["string"].asString() << std::endl;
                }
            }
        }
    }
    catch (...)
    {
        // 解析时一定要捕获异常，否则段错误
        return -1;
    }

    return 0;
}

static std::string WriteJson()
{
    Json::Value root;
    Json::Value obj_array;
    Json::Value item;
    Json::Value num_array;

    item["string"] = "this is a string";
    item["number"] = 999;
    item["aaaaaa"] = "bbbbbb";
    obj_array.append(item);

    item["string"] = "this is a string2";
    item["number"] = 9992;
    item["aaaaaa"] = "bbbbbb2";
    obj_array.append(item);

    // 直接对jsoncpp对象以数字索引作为下标进行赋值，则自动作为数组
    num_array[1] = 1;
    num_array[2] = 2;
    num_array[3] = 3;
    num_array[4] = 4;
    num_array[5] = 5;
    num_array[6] = 6;

    // 增加对象数组
    root["obj_array"] = obj_array;
    // 增加字符串
    root["name"] = "json";
    // 增加数字
    root["number"] = 666;
    // 增加布尔变量
    root["value"] = true;
    // 增加数字数组
    root["num_array"] = num_array;

    // 空对象
    Json::Value empty_item;
    empty_item["x"] = Json::Value();
    empty_item.clear();
    root["empty"] = empty_item;

    // 空数组
    Json::Value empty_array;
    empty_array.append(Json::Value());
    empty_array.clear();
    root["empty_array"] = empty_array;

    return root.toStyledString();
}

JsoncppTest::JsoncppTest()
{

}

JsoncppTest::~JsoncppTest()
{

}

void JsoncppTest::Test001()
{
    using namespace std;

    std::string msg;

    cout << "--------------------------------" << endl;
    msg = WriteJson();
    cout << "json write : " << endl << msg << endl;
    cout << "--------------------------------" << endl;
    cout << "json read :" << endl;
    ReadJson(msg);
    cout << "--------------------------------" << endl;
}

ADD_TEST_F(JsoncppTest, Test001);
