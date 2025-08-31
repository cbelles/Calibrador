#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include "Utils/json.hpp"

using json = nlohmann::json;
using namespace std;

class ExpulsionManager {
private:
    struct OutputConfig {
        int ticks_to_output;
    };

    std::unordered_map<int, OutputConfig> outputs;
    
    // Cola de prioridad usando min-heap (ordenado por tick de activación)
    using Activation = std::pair<int, int>; // (tick_activación, id_salida)
    std::priority_queue<Activation, std::vector<Activation>, std::greater<>> expulsion_queue;
    
    int current_tick = 0;

public:

    

    void addOutput(int output_id, int ticks_until_expulsion) {
        outputs[output_id] = {ticks_until_expulsion};
    }

    void tick() {
        current_tick++;
    }

    void scheduleExpulsion(int output_id) {
        int activation_tick = current_tick + outputs[output_id].ticks_to_output;
        //cout << "Scheduling expulsion for output " << output_id  
        //<< " at tick " << current_tick  << " + " <<  outputs[output_id].ticks_to_output << std::endl;
        expulsion_queue.emplace(activation_tick, output_id);
    }

    std::vector<int> getActiveOutputs() {
        std::vector<int> active;
        
        while (!expulsion_queue.empty() && 
               expulsion_queue.top().first <= current_tick) {
            active.push_back(expulsion_queue.top().second);
            expulsion_queue.pop();
        }
        
        return active;
    }

    bool loadPositionsFromFile(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                cout << "Error: Could not open file " << filename << std::endl;
                exit(0);  //return false;
            }

            json j;
            file >> j;

            if (!j.contains("positions") || !j["positions"].is_array()) {
                cout << "Error: Invalid JSON format" << std::endl;
                exit(0);  //return false;
            }

            outputs.clear();
            auto positions = j["positions"];

            //'''''''''''''''''''''''
            //print the loaded positions
            //cout << "Loaded positions from " << filename << ": ";
            //for (const auto& pos : positions) {
            //    cout << pos.get<int>() << " ";
            //}
            //cout << std::endl;
            //'''''''''''''''''''''''
            

            for (size_t i = 0; i < positions.size(); i++) {
                addOutput(i, positions[i].get<int>());
            }

            //'''''''''''''''''''''''
            //print std::vector<std::pair<int, int>> outputs
            //cout << "Outputs loaded: ";
            //for (const auto& output : outputs) {
            //    cout << "Output ID: " << output.first 
            //         << ", Ticks to Output: " << output.second.ticks_to_output <<  endl;;
            //}
            //cout << std::endl;
            //'''''''''''''''''''''''
            
            file.close();

            return true;

        } catch (const std::exception& e) {
            cout << "Error loading positions: " << e.what() << std::endl;
            return false;
        }
    }
};