/**
 * @file BlockQueueTest.cpp
 * @author jack
 * @brief 测试阻塞队列
 * @version 0.1
 * @date 2022-07-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <sstream>
#include <unistd.h>
#include "BlockQueue.h"

/*测试git*/
class TestBlockQueue{
public:
    TestBlockQueue(int threadnums = 4):jack_(std::make_shared<jack>()){
        jack_->bQueue_ = std::make_unique<BlockQueue<std::string>>(1000);
        jack_->isClose_ = false;
        for(int i = 0; i < 1; i++){
            jack_->threads_.emplace_back(std::make_unique<std::thread>([jack= jack_]{
                auto pid = std::this_thread::get_id();
                std::stringstream ss;
                ss << "test blockqueue thread id:  " << pid << "\n";
                while(true){
                    if(jack->isClose_){
                        break;
                    }else{
                        jack->bQueue_->push(ss.str());
                    }
                }
            }));
        }
        jack_->readThread_ = std::make_unique<std::thread>([jack= jack_]{
                auto pid = std::this_thread::get_id();
                std::stringstream ss;
                std::string s = "";
                while(true){
                    if(jack->isClose_){
                        break;
                    }else{
                        jack->bQueue_->pop(s);
                    }
                    ss<< s << pid << "\n";
                    std:: cout << ss.str();
                    if(s == "stop") {
                        jack->isClose_ = true;
                        break;
                    }
                }
        });
    }
    void joinAll(){
        for(auto & t: jack_->threads_){
            t->join();
        }
        for(int i= 0; i < jack_->threads_.size(); i++){
           jack_->bQueue_->push("stop"); 
        }
        jack_->readThread_->join();
        jack_->isClose_ = true;
    }
    
private:
    struct jack{
        std::atomic<bool> isClose_;
        std::unique_ptr<BlockQueue<std::string>> bQueue_;
        std::vector<std::unique_ptr<std::thread>> threads_;
        std::unique_ptr<std::thread> readThread_;
    };
    std::shared_ptr<jack> jack_;
    
};

int main(){
    TestBlockQueue test(4);
    test.joinAll();
}

