#include <iostream>
#include <fstream>
#include "json/json.h"          

void createJson();
bool parseJson(std::string& filename);

int main(int argc, char** argv){
    //createJson();
    std::string filename("test.json");
    parseJson(filename);

    return 0;
    
}

void createJson(){
    std::string jsonStr;
    Json::Value root, lang, mail;
    Json::StreamWriterBuilder writerBuilder;
    std::ofstream os("test.json");

    root["name"] = "Liming";
    root["Age"] = 26;

    lang[0] = "C++";
    lang[1] = "Java";
    root["Language"] = lang;

    mail["Netease"] = "lmshao@163.com";
    mail["Hotmail"] = "liming.shao@hotmail.com";
    root["E-mail"] = mail;

    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);

    return;
}

bool parseJson(std::string& filename){
    std::ifstream ifs;
    ifs.open(filename);
    if(!ifs.is_open()){
        std::cout << "open json file failed!" << std::endl;
        return 0;
    }
    bool res;
    Json::CharReaderBuilder readerBuilder;
    std::string err_json;
    Json::Value json_obj, lang, mail;
    try{
        bool ret = Json::parseFromStream(readerBuilder, ifs, &json_obj, &err_json);
        if(!ret){
            std::cout << "invalid json file " << filename << std::endl;
            return 0;
        }
    } catch(std::exception &e){
        std::cout << "exception while parse json file " << filename << ","
                  << e.what();
        return 0;
    }

    std::cout << "Name: " << json_obj["Name"].asString() << std::endl;
    std::cout << "Age: " << json_obj["Age"].asInt() << std::endl;

    lang = json_obj["Language"];
    std::cout << "Language: ";
    for(int i = 0; i < lang.size(); i++){
        std::cout << lang[i] << " ";
    }
    std::cout << std::endl;

    mail = json_obj["E-mail"];
    std::cout << "Netease: " << mail["Netease"] << std::endl;
    std::cout << "Hotmail: " << mail["Hotmail"] << std::endl;

    return true;
}

