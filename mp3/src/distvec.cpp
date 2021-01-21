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
#define INFINITY 10000
using namespace std;
typedef pair<int, int> PAIR;

map<pair<int,int> ,pair<int, int> >Init;
map<pair<int,int> ,pair<int, int> >Bell;//map<pair<source,dest>,pair<cost,next>>
map<int, map<int, int> >Neigh_info;  //map<source, map<neighbour, is_update>>
map<int, map<int, int> >Neigh_copy;
set<int> node;
typedef struct{
    int source;
    int dest;
    string text;
}msg;

typedef list<msg> msgList;
typedef list<int> path;

int need_update(){
    map<int, map<int, int> >::iterator it1;
    map<int, int>::iterator it2;
    for (it1 = Neigh_info.begin(); it1 != Neigh_info.end(); it1++){
        for (it2 = it1->second.begin(); it2 != it1->second.end(); it2++){
            if (it2->second == 1) {
              cout<<"need update:" <<"it1:"<<it1->first<<"  it2:"<<it2->first<<endl;
              return 1;
            }
        }
    }
    return 0;
}

int find_last(int source,int dest){
    path path_;
    if (dest == source) {
        return source;
    }
    PAIR temp_(source,dest);
    if(Bell[temp_].second == INFINITY){
        return INFINITY;
    }
    int next = Bell[temp_].second;
    if (next == dest){
        return source;
    } 
    path_.push_back(source);
    path_.push_back(next);
    int temp = next;
    PAIR temp_pair(next,dest);
    while(Bell[temp_pair].second!=dest){
        temp = Bell[temp_pair].second;
        path_.push_back(temp);
        temp_pair.first = temp;
    } 
    return temp;
}


void update_until_converge(){
    map<int, map<int, int> >::iterator it1;
    map<int, int>::iterator it2;
    set<int>::iterator it3;
    map<int, int>::iterator it4;
    cout<<"(need_update()"<<need_update()<<endl;
  
    while (need_update() != 0){
        for (it1 = Neigh_info.begin(); it1 != Neigh_info.end(); it1++){
            for (it2 = it1->second.begin(); it2 != it1->second.end(); it2++){
                if (it2->second != 0){
                    for (it3 = node.begin(); it3 != node.end(); it3++){
                        PAIR p2(it2->first, *it3);
			PAIR p1(it1->first, it2->first);
                        PAIR p3(it1->first, *it3);
                        PAIR p4( it2->first,it1->first);
                       
                        cout<<"p2:"<<it2->first<<" p1:"<<it1->first<<" p3:"<<*it3<<endl;
      
                        if ((Bell[p1].first + Bell[p3].first < Bell[p2].first) || ((Bell[p1].first + Bell[p3].first == Bell[p2].first) && 
                            (it1->first < Bell[p2].second))){
                            if(*it3 == it1->first && Bell[p1].first + Bell[p3].first<Init[p2].first) continue;
	                    Bell[p2].first = Bell[p1].first + Bell[p3].first;
                         
	                    Bell[p2].second = find_last(it1->first,it2->first);
                            //Bell[p2].second = it1->first;
	                    cout<<"it1->first:"<<it1->first<<endl;
	                    cout<<"Bell_step:"<<endl;
			    cout << left;
			    for (int i=1;i<=node.size();i ++){
				for (int j=1;j<=node.size();j ++){
					PAIR print(i,j);
					cout << "cost:"<<Bell[print].first<<" "<<"next:"<<Bell[print].second<<"   ";
		 	 	}
		 	 	cout<<endl;
			    } 
	                    for (it4 = Neigh_info[it2->first].begin(); it4 != Neigh_info[it2->first].end(); it4++){
	                        it4->second = 1;
	                    }
                             
                        }
                        /*cout<<"Bell_step:"<<endl;
		    	cout << left;
		    	for (int i=1;i<=node.size();i ++){
			for (int j=1;j<=node.size();j ++){
			    PAIR print(i,j);
			    cout << "cost:"<<Bell[print].first<<" "<<"next:"<<Init[print].second<<"   ";
		     	 }
		     	 cout<<endl;
		    	} */
                    }
                    it2->second = 0;
                }

            }
        }
      	
       
    }
    cout<<"Bell1:"<<endl;
    	cout << left;
    	for (int i=1;i<=node.size();i ++){
        for (int j=1;j<=node.size();j ++){
            PAIR print(i,j);
	    cout << "cost:"<<Bell[print].first<<" "<<"next:"<<Bell[print].second<<"   ";
     	 }
     	 cout<<endl;
    	}

}




path find_hops(int source,int dest, int next){
    path path_;
    if (dest == source) {
        path_.push_back(dest);
        return path_;
    }
    if (next == dest){
        path_.push_back(source);
        path_.push_back(next);
        return path_;
    } 
    path_.push_back(source);
    path_.push_back(next);
    int temp = next;
    PAIR temp_pair(next,dest);
    while(temp!=dest){
        temp = Bell[temp_pair].second;
        path_.push_back(temp);
        temp_pair.first = temp;
    }  
    return path_;
}

void load_Bell(){
    Bell.clear();
    set<int>::iterator it1;
    set<int>::iterator it2;
    PAIR temp(0, 0);
    PAIR costpair(0, 0);
    PAIR zero(0,0);
    for (it1 = node.begin(); it1 != node.end(); it1++){
        for (it2 = node.begin(); it2 != node.end(); it2++){
            if (*it1 == *it2){
                temp.first = *it1;
                temp.second = *it1;
                costpair.first = 0;
                costpair.second = *it1;
                Bell[temp] = costpair;
                
                continue;
                
            }
            PAIR temp2(*it1, *it2);
            PAIR temp3(*it2, *it1);
            if (Init.count(temp2) == 0){                           //????.count
                Bell[temp2].first = INFINITY;
                Bell[temp2].second = INFINITY;
                Bell[temp3].first = INFINITY;
                Bell[temp3].second = INFINITY;
    
                continue;
                
            }
            if (Init.count(temp2) != 0){
                Bell[temp2] = Init[temp2];
                Bell[temp3] = Init[temp3];
            }
            
        }
    }
    cout<<"Bell:"<<endl;
    cout << left;
    for (int i=1;i<=node.size();i ++){
        for (int j=1;j<=node.size();j ++){
            PAIR print(i,j);
	    cout << "cost:"<<Bell[print].first<<" "<<"next:"<<Bell[print].second<<"   ";
      }
      cout<<endl;
    }
}

void write_to_file(FILE* fpOut,msgList input_msgs){
    list<msg>::iterator it;
    list<int>::iterator it1;
    PAIR temp(0,0);
    path path_;
    PAIR inf_(INFINITY,INFINITY);
    for(int i=1;i<=node.size();i ++){
        for(int j=1;j<=node.size();j ++){
            temp.first = i;
            temp.second = j;
            if (Bell[temp]!=inf_){
    		path_ = find_hops(i,j,Bell[temp].second);
    		fprintf(fpOut,"%d %d %d\n",j,Bell[temp].second,Bell[temp].first);
            }
        }
    }
    
    for(it =input_msgs.begin();it!= input_msgs.end();it++){
        
            PAIR s_d(it->source,it->dest);
            if (Bell[s_d] == inf_){
                fprintf(fpOut, "from %d to %d cost infinite hops unreachable message %s", it->source,it->dest, it->text.c_str());
                continue;
            }
            fprintf(fpOut, "from %d to %d cost %d ", it->source,it->dest,Bell[s_d].first);
            path path1;
            path1 = find_hops(it->source,it->dest,Bell[s_d].second);
            fprintf(fpOut,"hops");
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
        printf("Usage: ./distvec topofile messagefile changesfile\n");
        return -1;
    }

    //load files

    ifstream topo_file(argv[1]);
    ifstream msg_file(argv[2]);
    ifstream topo_change(argv[3]);



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


    FILE *fpOut;
    fpOut = fopen("output.txt", "w");
     
    //read graph from topology file
    int node1,node2,cost;
    PAIR temp(0,0);
    PAIR costpair(0,0);
    while (topo_file >> node1 >> node2 >> cost){
		temp.first = node1;
        temp.second = node2;
        costpair.first = cost;
        costpair.second = node2;
        Init[temp] = costpair;
        temp.second = node1;
        temp.first = node2;
        costpair.first = cost;
        costpair.second = node1;
	Init[temp] = costpair;
	node.insert(node1);
	node.insert(node2);
        Neigh_info[node1][node2] = 1;
        Neigh_info[node2][node1] = 1;
	Neigh_copy[node1][node2] = 1;
        Neigh_copy[node2][node1] = 1;
	}
     for (int i=1;i<=node.size();i ++){
           PAIR same(i,i);
           same.first = i;
           same.second = i;
           costpair.first = 0;
           costpair.second = i;
	   Init[same] = costpair;
      }

/*cout<<"Init:"<<endl;
    cout << left;
    for (int i=1;i<=node.size();i ++){
        for (int j=1;j<=node.size();j ++){
            PAIR print(i,j);
	    cout << "cost:"<<Init[print].first<<" "<<"next:"<<Init[print].second<<"   ";
      }
      cout<<endl;
    }
    for (int i=1;i<=node.size();i ++){
       for (int j=1;j<=node.size();j ++){
    		cout << i<<" neighbor info"<< j <<"   value:"<<Neigh_info[i][j]<<endl;
        }
    }*/
    load_Bell();
    update_until_converge();

    cout<<"Bell_print:"<<endl;
    cout << left;
    for (int i=1;i<=node.size();i ++){
        for (int j=1;j<=node.size();j ++){
            PAIR print(i,j);
	    cout << "cost:"<<Bell[print].first<<" "<<"next:"<<Bell[print].second<<"   ";
      }
      cout<<endl;
    }

    write_to_file(fpOut, input_msgs);

    while (topo_change >> node1 >> node2 >> cost){
	Neigh_info.clear();
        if (cost != -999){
            temp.first = node1;
            temp.second = node2;
            costpair.first = cost;
            costpair.second = node2;
            Init[temp] = costpair;
            temp.second = node1;
            temp.first = node2;
            costpair.first = cost;
            costpair.second = node1;
            Init[temp] = costpair;
            node.insert(node1);
            node.insert(node2);
            Neigh_copy[node1][node2] = 1;
            Neigh_copy[node2][node1] = 1;
        }
        else {
            temp.first = node1;
            temp.second = node2;
            Init.erase(temp);
            temp.second = node1;
            temp.first = node2;
            Init.erase(temp);
            Neigh_copy[node1].erase(node2);
            Neigh_copy[node2].erase(node1);
        }
	map<int, map<int, int> >::iterator st;
	map<int, int>:: iterator de;
	for (st = Neigh_copy.begin(); st != Neigh_copy.end(); st++){
	    for (de = st->second.begin(); de != st->second.end(); de++){
		Neigh_info[st->first][de->first] = 1;
	    }
	}
        load_Bell();
        update_until_converge();
        write_to_file(fpOut, input_msgs);
    }




    fclose(fpOut);

    return 0;
}
