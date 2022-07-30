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
    TestBlockQueue(int threadnums = 4):jack_(std::make_shared<jack>()),threadnums_(threadnums) {
        jack_->bQueue_ = std::make_unique<BlockQueue<std::string>>(1000);
        jack_->isClose_ = false;
        for(int i = 0; i < threadnums_; i++){
            jack_->threads_.emplace_back(std::make_unique<std::thread>([jack= jack_]{
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
                    if(s == "stop"){
                        break;
                    }
                }
            }));
        }
    }
    void joinAll(){
        for(int i=0; i < threadnums_; i++){
            jack_->bQueue_->push("stop");
        }
        for(auto & t: jack_->threads_){
            t->join();
        }
    }
    void run(int times){
        auto pid = std::this_thread::get_id();
        std::stringstream ss;
        ss << "test blockqueue thread id:  " << pid << "\n";
        for(int i = 0; i < times; i++){
            jack_->bQueue_->push(ss.str());
        }
    }
    
private:
    struct jack{
        std::atomic<bool> isClose_;
        std::unique_ptr<BlockQueue<std::string>> bQueue_;
        std::vector<std::unique_ptr<std::thread>> threads_;
    };
    std::shared_ptr<jack> jack_;
    int threadnums_;
    
};

int main(){
    TestBlockQueue test(4);
    test.run(1000);
    test.joinAll();
}

