#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <list>
#include <iomanip>
using namespace std;

#define square(x) x*x

typedef list<int> R;
typedef pair<int, int> PAIR;
typedef pair<int, PAIR> TRIP;

int idle;                   //the channel is idle or not (idle = 1; busy = 0)
int collision;
int time_ = 0;                   //the current time unit

int node_num;
int pkt_length;
R   R_list;
int R_len;
int retrans_max;
int total_time;

int Channel_utilization;
int Channel_idle_fraction;      //these are recorded in the unit of "time unit"
int total_collisions;

float variance_succ_trans;
float variance_coll;

TRIP Node_info; // pair<count_down, pair<succ, coll> >
map<int, TRIP> Nodes; // collection<node_index, node_info>
map<int, int> time_still_need; //map<node_index, how_many_time_units_still_need>
map<int, PAIR> R_coll; //map<node_index, pair<current_R, collision>>

float calculate_variance_succ_trans(){
    float result;
    float sum = 0.0;
    float average;
    map<int, TRIP>::iterator it;
    for (it = Nodes.begin(); it != Nodes.end(); it++){
        sum += (it->second).second.first;
    }
    average = sum/node_num;

    for (it = Nodes.begin(); it != Nodes.end(); it++){
        result += square(((it->second).second.first - average))/node_num;
    }

    return result;
}

float calculate_variance_coll(){
    float result;
    float sum = 0.0;
    float average;
    map<int, TRIP>::iterator it;
    for (it = Nodes.begin(); it != Nodes.end(); it++){
        sum += (it->second).second.second;
    }
    average = sum/node_num;

    for (it = Nodes.begin(); it != Nodes.end(); it++){
        result += square(((it->second).second.second - average))/node_num;
    }

    return result;
}

void update_R(int node_index){
    if (R_coll[node_index].second == 6){   // map<node_index, pair<current_R, collision>>
        R_coll[node_index].second = 0;
        R_coll[node_index].first = R_list.front();
    }
    else {
        list<int>::iterator it;              //update R upper
        for (it = R_list.begin(); it != R_list.end(); it++){
            if (*it > R_coll[node_index].first){
                R_coll[node_index].first = *it;
                break;
            }
        }
    }
    return;
}

int judge_state(){
    int first_zero_index = -1;
    int zero_appears = 0;        //if count down becomes 0
    collision = 0;
    map<int, TRIP>::iterator it;
    for (it = Nodes.begin(); it != Nodes.end(); it++){  // nodes<node_index, node_info>  nodeinfo: pair<count_down, pair<succ, coll> >
        if ((it->second).first == 0){       //count down =0
            if (zero_appears == 0){         //first ready to send node     
                zero_appears = 1;           
                first_zero_index = it->first;  //store the first ready node
            }
            //one ready nodes appears,collision happens, two ready nodes
            //it->first ==node_index
            else if(collision == 0){
                R_coll[first_zero_index].second += 1;   //increment of collision count of first ready node
                update_R(first_zero_index);
                Nodes[first_zero_index].first = rand()%(R_coll[first_zero_index].first );

                R_coll[it->first].second += 1;
                update_R(it->first);
                (it->second).first = rand()%(R_coll[it->first].first );
                //set the collision state of two ready nodes to 1
                Nodes[first_zero_index].second.second += 1;
                (it->second).second.second += 1;
                collision = 1;
            }
            //more than two ready nodes,update current new ready node
            else{
                R_coll[it->first].second += 1;
                update_R(it->first);
                (it->second).first = rand()%(R_coll[it->first].first);
                (it->second).second.second += 1;
            }
        }
    }
    idle = 1 - zero_appears;
    return first_zero_index;
}

int simulation(){

    map<int, TRIP>::iterator it;
    list<int>::iterator listit;
    PAIR two_zeros(0, 0);
    TRIP three_zeros(0, two_zeros);//TRIP pair<count_down, pair<succ, coll> >
    //Initialization
    for (int i = 0; i < node_num; i++){
        three_zeros.first = rand()%(R_list.front());
        Nodes[i] = three_zeros;
        time_still_need[i] = 0;
        R_coll[i].first = R_list.front();
        R_coll[i].second = 0;
    }
    
    int first_zero_index;
    int i =0;
    for (time_ = 0; time_ < total_time; time_++){
        first_zero_index = judge_state();
        i+=1;
        if (collision) {
            total_collisions++;
            continue;
        }


        if (idle){
            for (it = Nodes.begin(); it != Nodes.end(); it++){
                (it->second).first -= 1;
            }
            Channel_idle_fraction += 1;
        }

        else {
            if (time_still_need[first_zero_index] == 0){
                time_still_need[first_zero_index] = pkt_length;
            }
            else {
                time_still_need[first_zero_index] -= 1;
            }
            if (time_still_need[first_zero_index] == 0){
                R_coll[first_zero_index].first = R_list.front();
                Nodes[first_zero_index].first = rand()%(R_coll[first_zero_index].first);
                R_coll[first_zero_index].second = 0; // map<node_index, pair<current_R, collision>>
                Nodes[first_zero_index].second.first += 1;//success
            }
            Channel_utilization += 1;
        }

    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: ./csma input.txt\n");
        return -1;
    }

    ifstream input_file(argv[1]);

    char C;
    int Random_upper;

    input_file>> C >> node_num >> C >> pkt_length>>C;
    //cout<<"C"<<C<<endl;
    while(input_file.get()!='\n'){
        input_file>>Random_upper;
       // cout<<"Random_upper"<<Random_upper<<endl;
        R_list.push_back(Random_upper);
    }
    R_len = R_list.size();
    input_file>> C >> retrans_max >> C >> total_time;
    /*cout<<"node_num:"<<node_num<<endl;
    cout<<"pkt_length:"<<pkt_length<<endl;
    list<int>::iterator it5;
    for ( it5= R_list.begin(); it5 != R_list.end(); it5++){
               cout<<*it5<<endl;
           }
    cout<<"retrans_max:"<<retrans_max<<endl;
    cout<<"total_time:"<<total_time<<endl;*/



    simulation();
    variance_succ_trans = calculate_variance_succ_trans();
    variance_coll = calculate_variance_coll();
    //write to output file below this line

    FILE *fpOut;
    fpOut = fopen("output.txt", "w");
    fprintf(fpOut, "Channel utilization (in percentage) %f\n", ((float)Channel_utilization / total_time) * 100.0);
    fprintf(fpOut, "Channel idle fraction (in percentage) %f\n",((float)Channel_idle_fraction / total_time) * 100.0);
    fprintf(fpOut, "Total number of collisions %d\n", total_collisions);
    fprintf(fpOut, "Variance in number of successful transmissions (across all nodes) %f\n", variance_succ_trans);
    fprintf(fpOut, "Variance in number of collisions (across all nodes) %f\n", variance_coll);

    return 0;

}
