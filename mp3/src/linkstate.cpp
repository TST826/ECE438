#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<iostream>
#include<fstream>
#include <iostream>
#include <map>
#include <set>
#include <list>
#include <iomanip>
using namespace std;

#define INFINITY 30000

typedef pair<int, int>PAIR;

map<pair<int, int>, int>Init;  //pair<source,dest>, cost
map<pair<int,int> ,pair<int, int> >Dijk;//pair<source,dest>,pair<cost,last>
set<int> node;
typedef struct{
    int source;
    int dest;
    string text;
}msg;

map<int, pair<int, int> > already;//dest,pair<cost,last>
map<int, pair<int, int> > total;//dest,pair<cost,last>

typedef list<msg> msgList;
typedef list<int> path;
/*PAIR temp(1,2);
PAIR temp1(3,5);
for (int i = 0; i < 10; i++){
   temp.first = i;
   Dijk[temp] = temp1;
}
for (int i = 0; i < 10; i++){
   temp.first = i;
   cout << Dijk[temp].second << endl;
}*/



int update_weights(int node_idx){
    set<int>::iterator it3;
    for (it3 = node.begin(); it3 != node.end(); it3++){             //iterate all nodes
        PAIR temp2(node_idx, *it3);
        PAIR temp3(0,0);
        if ((*it3 != node_idx) && (Init[temp2]!= 0)&&already.count(*it3)==0){        //if there is a edge between node *it3 and node_idx(current already node)
            //cout<<"*it3:"<<*it3<<endl;
            if ((already[node_idx].first + Init[temp2] < total[*it3].first) || ((already[node_idx].first + Init[temp2] == total[*it3].first) 
                && (node_idx <= total[*it3].second))){
                //cout<<"already[node_idx].first + Init[temp2]"<<already[node_idx].first + Init[temp2]<<endl;
                //cout<<"total[*it3].first"<<total[*it3].first<<endl;
                total[*it3].first = min(already[node_idx].first + Init[temp2], total[*it3].first);//update cost of all remaining unalready nodes
                total[*it3].second = node_idx;
            }
        }
    }
    return already[node_idx].first;
}


int find_lowest_weight(int current_node){
    /*cout<<"total:"<<endl;
    map<int, pair<int, int> >::iterator it5;
            for ( it5= total.begin(); it5 != total.end(); it5++){
               cout<<setw(12)<<"index:"<<it5->first<<setw(3)<<"cost:"<<it5->second.first<<setw(3)<<"last"<<it5->second.second<<endl;
           }*/
    int temp_cost = INFINITY;
    int temp_last = 0;
    int node_idx = INFINITY;
    map<int, pair<int, int> >::iterator it4;
    for ( it4= total.begin(); it4 != total.end(); it4++){
        if(it4->second.first < temp_cost){
            node_idx = it4->first;
            temp_cost = it4->second.first;
            temp_last = it4->second.second;
        }
        else if(it4->second.first == temp_cost){
            if(it4->first < node_idx){
		    node_idx = it4->first;
		    temp_cost = it4->second.first;
		    temp_last = it4->second.second;
            }
        }
    }
    //cout<<"temp last"<<temp_last<<endl;
    if (temp_cost == INFINITY) return -1;

    PAIR temp3(temp_cost, temp_last);
    already[node_idx]=temp3;
    PAIR temp4(current_node, node_idx);
    PAIR temp5(temp_cost, temp_last);
    Dijk[temp4] = temp5;

    return node_idx;
}

void build(){
    Dijk.clear();
    already.clear();
    total.clear();
    set<int>::iterator it;
    set<int>::iterator it2;

    PAIR temp(0,0);
    PAIR cost_last(0,0);
    int shortest = 0;
    int node_idx;
    for (it = node.begin(); it != node.end(); it++){                //iterate all the node
        total.clear();
        for (it2 = node.begin(); it2 != node.end(); it2++){        //for each node,initialize all  dest, <inf,0>
            PAIR tempp(INFINITY, 0);
            total[*it2] = tempp;
        }
        already.clear();
        temp.first = 0;
        temp.second = *it;
        already[*it] = temp;             // store already  source,<0,source>
        temp.first = *it;
        temp.second = *it;
        cost_last.first = 0;
        cost_last.second = *it;
        Dijk[temp] = cost_last;           // store Dijk  <source,source><0,source>
         map<int, pair<int, int> >::iterator it5;
       /*cout<<"total:"<<endl;
            for ( it5= total.begin(); it5 != total.end(); it5++){
               cout<<setw(12)<<"index:"<<it5->first<<setw(3)<<"cost:"<<it5->second.first<<setw(3)<<"last"<<it5->second.second<<endl;
           }*/
        update_weights(*it);
        total.erase(*it);
         /*cout<<"total:"<<endl;
            for ( it5= total.begin(); it5 != total.end(); it5++){
               cout<<setw(12)<<"index:"<<it5->first<<setw(3)<<"cost:"<<it5->second.first<<setw(3)<<"last"<<it5->second.second<<endl;
           }*/
        while (already.size() != node.size()){
            node_idx = find_lowest_weight(*it);
            //cout<<"node_idx:"<<node_idx<<endl;
            total.erase(node_idx);              //update remaining nodes
            if (node_idx == -1){                 //????????????????????????????????????????????//
                break;
            }
            shortest = update_weights(node_idx);
            /*cout<<"total:"<<endl;
            for ( it5= total.begin(); it5 != total.end(); it5++){
               cout<<setw(12)<<"index:"<<it5->first<<setw(3)<<"cost:"<<it5->second.first<<setw(3)<<"last"<<it5->second.second<<endl;
            }*/
	    

        }
        /*cout<<"already:"<<endl;
        for ( it5= already.begin(); it5 != already.end(); it5++){
            cout<<setw(12)<<"index:"<<it5->first<<setw(3)<<"cost:"<<it5->second.first<<setw(3)<<"last"<<it5->second.second<<endl;
        }*/
 
    }
    //print Dijk table
    cout << "Dijk"<<endl;
    cout << left;
    for (int i=1;i<=node.size();i ++){
        for (int j=1;j<=node.size();j ++){
            PAIR print(i,j);
	    cout<<setw(6)<<"cost:"<<Dijk[print].first<<"last:"<<Dijk[print].second<<"  ";
      }
      cout<<endl;
    }
}

pair<path,int> find_hops(int source,int dest, int last){
    pair<path,int> pair1;
    path path_;
    path_.push_back(dest);
    if (dest == source) {
        pair1.first = path_;
        pair1.second = source;
        return pair1;
    }
    path_.push_back(last);
    if (last == source){
        pair1.first = path_;
        pair1.second = dest;
        return pair1;
    } 
    int temp = last;
    PAIR temp_pair(source,last);
    int next;
    while(temp!=source){
        if(Dijk[temp_pair].second == source){
            next = temp_pair.second;
        }
        temp = Dijk[temp_pair].second;
        path_.push_back(temp);
        temp_pair.second = temp;
    }  
    pair1.first = path_;
    pair1.second = next;
    return pair1;
}

void write_to_file(FILE* fpOut,msgList input_msgs){
    list<msg>::iterator it;
    list<int>::iterator it1;
    PAIR temp(0,0);
            pair<path, int> path_;
            int next;
            PAIR inf_(0,0);
            for(int i=1;i<=node.size();i ++){
                for(int j=1;j<=node.size();j ++){
                    temp.first = i;
                    temp.second = j;
                    if (Dijk[temp]!=inf_){
            		path_ = find_hops(i,j,Dijk[temp].second);
            		next = path_.second;
            		fprintf(fpOut,"%d %d %d\n",j,next,Dijk[temp].first);
                    }
                }
            }
            
    for(it =input_msgs.begin();it!= input_msgs.end();it++){
        
            PAIR s_d(it->source,it->dest);
            if (Dijk[s_d] == inf_){
                fprintf(fpOut, "from %d to %d cost infinite hops unreachable message %s", it->source,it->dest, it->text.c_str());
                continue;
            }
            fprintf(fpOut, "from %d to %d cost %d ", it->source,it->dest,Dijk[s_d].first);
            path path1;
            path_ = find_hops(it->source,it->dest,Dijk[s_d].second);
            path1 = path_.first;
            fprintf(fpOut,"hops");
            path1.reverse();
            path1.remove(it->dest);
            for (it1 =path1.begin();it1!= path1.end();it1++){
                fprintf(fpOut," %d",*it1);
            }
            fprintf(fpOut, " message%s\n", it->text.c_str());
           
    }
}



int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    }

    ifstream topo_file(argv[1]);
    ifstream msg_file(argv[2]);
    ifstream topo_change(argv[3]);
    Init.clear();
    //read graph from topology file
    int node1,node2,cost,change_cost;
    PAIR temp(0,0);
    while (topo_file >> node1 >> node2 >> cost)
	{
		temp.first = node1;
        	temp.second = node2;
        	Init[temp] = cost;
                temp.second = node1;
        	temp.first = node2;
		Init[temp] = cost;
		//cout << Init[temp] << endl;
		node.insert(node1);
		node.insert(node2);
	}
    //print the Init graph
    cout << left;
    for (int i=1;i<=node.size();i ++){
        for (int j=1;j<=node.size();j ++){
            PAIR print(i,j);
	    cout<<setw(12)<<Init[print];
      }
      cout<<endl;
    }

  /*& set<int>::iterator it;
    for(it=node.begin();it!=node.end();it++){
        printf("%d\n",*it);
    }*/
    //read message from msg file
    msgList input_msgs;
    msg input_msg;
    string input_text;
    int src,dst;
    while(msg_file>>src>>dst){
        input_msg.source = src;
        input_msg.dest = dst;
        getline(msg_file,input_text);
        input_msg.text = input_text;
        input_msgs.push_back(input_msg);
    }
    /*list<msg>::iterator it; 
    for(it =input_msgs.begin();it!= input_msgs.end();it++){
	cout<<it->source<<endl;
 	cout<<it->dest<<endl;        
	cout<<it->text<<endl;
    }*/
    
    build();
    FILE *fpOut;
    fpOut = fopen("output.txt", "w");
    write_to_file(fpOut,input_msgs);
    
    while (topo_change >> node1 >> node2 >> change_cost)
	{
		if (change_cost == -999){
                   temp.first = node1;
        	   temp.second = node2;
                   Init.erase(temp);
                   temp.second = node1;
        	   temp.first = node2;
                   Init.erase(temp);
                }
                else{
                   	temp.first = node1;
			temp.second = node2;
			Init[temp] = change_cost;
		        temp.second = node1;
			temp.first = node2;
			Init[temp] = change_cost;
			node.insert(node1);
			node.insert(node2);
                }
   /*cout<<"________________________________________"<<endl;
                 cout << left;
    for (int i=1;i<=node.size();i ++){
        for (int j=1;j<=node.size();j ++){
            PAIR print(i,j);
	    cout<<setw(12)<<Init[print];
      }
      cout<<endl;
    }*/
                build();
                write_to_file(fpOut,input_msgs);
	}



    fprintf(fpOut,"\n");
    fclose(fpOut);
    

    return 0;

}
