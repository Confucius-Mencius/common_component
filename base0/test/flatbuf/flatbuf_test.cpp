#include "msg_generated.h"
#include <iostream>
#include "flatbuffers/idl.h"

// 更多接口参考：http://google.github.io/flatbuffers/flatbuffers_guide_tutorial.html

using namespace com::moon::game;

int main()
{
    // 常规使用只需要#include "msg_generated.h"即可，不用链接libflatbuffers.so
    flatbuffers::FlatBufferBuilder builder;

    /////////// Serialize //////////
    int d[10];
    for (int i = 0; i < 10; ++i)
    {
        d[i] = i;
    }

    auto field_d = builder.CreateVector(d, sizeof(d) / sizeof(d[0]));

    auto weapon_one_name = builder.CreateString("Sword");
    short weapon_one_damage = 3;
    auto weapon_two_name = builder.CreateString("Axe");
    short weapon_two_damage = 5;
// Use the `CreateWeapon` shortcut to create Weapons with all the fields set.
    auto sword = ddz::CreateWeapon(builder, weapon_one_name, weapon_one_damage);
    auto axe = ddz::CreateWeapon(builder, weapon_two_name, weapon_two_damage);

// Serialize a name for our monster, called "Orc".
    auto name = builder.CreateString("Orc");
// Create a `vector` representing the inventory of the Orc. Each number
// could correspond to an item that can be claimed after he is slain.
    unsigned char treasure[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto inventory = builder.CreateVector(treasure, 10);

// Place the weapons into a `std::vector`, then convert that into a FlatBuffer `vector`.
    std::vector<flatbuffers::Offset<ddz::Weapon>> weapons_vector;
    weapons_vector.push_back(sword);
    weapons_vector.push_back(axe);
    auto weapons = builder.CreateVector(weapons_vector);

// Set his hit points to 300 and his mana to 150.
    int hp = 300;
    int mana = 150;
// Finally, create the monster using the `CreateMonster` helper function
// to set all fields.
    ddz::Vec3 pos = ddz::Vec3(1.0f, 2.0f, 3.0f);
    auto orc = CreateMonster(builder, &pos, mana, hp, name,
                             inventory, ddz::Color_Red, weapons, ddz::Equipment_Weapon,
                             axe.Union());

    auto req1 = CreateReq1(builder, 100, 200, builder.CreateString("abc"), field_d, orc);
    auto msg = ddz::CreateMsg(builder, ddz::MsgBody_Req1, req1.Union());
    builder.Finish(msg);

    // get data and size
    auto data_len = builder.GetSize();
    std::cout << data_len << std::endl;

    uint8_t* data = builder.GetBufferPointer();
    char* buf = new char[data_len + 1];
    memcpy(buf, data, data_len);
    std::cout << buf << std::endl;
    delete[] buf;

    ////////// Deserialize //////////
    ////////// 收到数据parse之前要先verify，免得crash掉 //////////
    auto verifier = flatbuffers::Verifier(data, data_len);
    if (!ddz::VerifyMsgBuffer(verifier))
    {
        std::cout << "invalid data" << std::endl;
        return -1;
    }

    auto req1_msg = ddz::GetMsg(data);
    if (nullptr == req1_msg)
    {
        return -1;
    }

    assert(ddz::MsgBody_Req1 == req1_msg->msg_body_type());
    auto req1_d = static_cast<const ddz::Req1*>(req1_msg->msg_body());
    std::cout << req1_d->a() << std::endl;
    std::cout << req1_d->b() << std::endl;

    std::cout << req1_d->c()->c_str() << std::endl;
    std::cout << req1_d->c()->Length() << std::endl;

    std::cout << req1_d->c()->data() << std::endl;
    std::cout << req1_d->c()->size() << std::endl;

    for (auto it = req1_d->d()->begin(); it != req1_d->d()->end(); ++it)
    {
        std::cout << *it << std::endl;
    }

    auto monster = req1_d->monster();
    if (monster != nullptr)
    {
        std::cout << monster->hp() << std::endl;
        std::cout << monster->mana() << std::endl;
        std::cout << monster->name()->c_str() << std::endl;
    }

    auto pos_d = monster->pos();
    if (pos_d != nullptr)
    {
        auto x = pos_d->x();
        auto y = pos_d->y();
        auto z = pos_d->z();
    }

//Similarly, we can access elements of the inventory vector by indexing it. You can also iterate over the length of the array/vector representing the FlatBuffers vector.
    auto inv = monster->inventory(); // A pointer to a `flatbuffers::Vector<>`.
    auto inv_len = inv->Length();
    auto third_item = inv->Get(2);

    // For vectors of tables, you can access the elements like any other vector, except your need to handle the result as a FlatBuffer table:
    auto weapons_d = monster->weapons(); // A pointer to a `flatbuffers::Vector<>`.
    auto weapon_len = weapons_d->Length();
    auto second_weapon_name = weapons_d->Get(1)->name()->str();
    auto second_weapon_damage = weapons_d->Get(1)->damage();


//    Last, we can access our Equipped FlatBuffer union. Just like when we created the union, we need to get both parts of the union: the type and the data.
//        We can access the type to dynamically cast the data as needed (since the union only stores a FlatBuffer table).
    auto union_type = monster->equipped_type();
    if (union_type == ddz::Equipment_Weapon)
    {
        auto weapon = static_cast<const ddz::Weapon*>(monster->equipped()); // Requires `static_cast`
        // to type `const Weapon*`.
        auto weapon_name = weapon->name()->str(); // "Axe"
        auto weapon_damage = weapon->damage();    // 5
    }

    {
        // 二进制与json之间的转换，需要头文件
        // #include "flatbuffers/idl.h"
        // #include "flatbuffers/util.h"
        // 且要链接libflatbuffers.so

        // 首先要加载schema文件
        std::string schemafile;
        if (!flatbuffers::LoadFile("/home/hgc/workspace/mine/hilton/c_common_component/base/test/flatbuf/msg.fbs",
                                   false, &schemafile))
        {
            return -1;
        }

        flatbuffers::Parser parser;
        if (!parser.Parse(schemafile.c_str()))
        {
            return -1;
        }

//        将二进制转化为json
        std::string jsongen;
        GenerateText(parser, data, &jsongen);
        std::cout << jsongen << std::endl;

        for (int i = 0; i < 2; ++i)
        {
            // 将json转换为二进制
            if (!parser.Parse(jsongen.c_str()))
            {
                return -1;
            }

            flatbuffers::FlatBufferBuilder& fbb = parser.builder_;

            // get data and size
            auto data_len = fbb.GetSize();
            std::cout << data_len << std::endl;

            uint8_t* data = fbb.GetBufferPointer();
            std::string s;
            s.assign((char*) data, data_len);
            //char buf[data_len + 1] = "";
            //memcpy(buf, data, data_len);
            std::cout << s << std::endl;
        }
    }

    {
//        Storing maps / dictionaries in a FlatBuffer
//
//        FlatBuffers doesn't support maps natively, but there is support to emulate their behavior with vectors and binary search, which means you can have fast lookups directly from a FlatBuffer without having to unpack your data into a std::map or similar.
//
//        To use it:
//
//        Designate one of the fields in a table as they "key" field. You do this by setting the key attribute on this field, e.g. name:string (key). You may only have one key field, and it must be of string or scalar type.
//            Write out tables of this type as usual, collect their offsets in an array or vector.
//            Instead of CreateVector, call CreateVectorOfSortedTables, which will first sort all offsets such that the tables they refer to are sorted by the key field, then serialize it.
//            Now when you're accessing the FlatBuffer, you can use Vector::LookupByKey instead of just Vector::Get to access elements of the vector, e.g.: myvector->LookupByKey("Fred"), which returns a pointer to the corresponding table type, or nullptr if not found. LookupByKey performs a binary search, so should have a similar speed to std::map, though may be faster because of better caching. LookupByKey only works if the vector has been sorted, it will likely not find elements if it hasn't been sorted.
//
    }

    return 0;
}
