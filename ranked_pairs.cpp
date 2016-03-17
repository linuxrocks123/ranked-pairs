#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <set>
#include <vector>

using namespace std;

struct Head2Head
{
     int first;
     int second;
     int score;
};

struct GraphNode
{
     int id;
     set<int> edges;
};

bool is_reachable(const vector<GraphNode>& graph, int origin, int destination)
{
     for(int x : graph[origin].edges)
     {
          if(is_reachable(graph,x,destination))
               return true;
     }

     return false;
}

int main()
{
     string temp;

     cout << "Enter number of candidates: ";
     getline(cin,temp);
     int candnum = stoi(temp);
     
     vector<string> candidates;
     for(int i=1; i<=candnum; i++)
     {
          cout << "Enter candidate #" << i << ": ";
          getline(cin,temp);
          while(find(candidates.begin(),candidates.end(),temp)!=candidates.end())
          {
               cout << "That candidate has been included already." << endl;
               cout << "Enter candidate #" << i;
               getline(cin,temp);
          }
          candidates.push_back(temp);
     }

     cout << "Enter number of ballots: ";
     getline(cin,temp);
     int ballots_size = stoi(temp);

     vector<vector<set<int>>> ballots;
     ballots.reserve(ballots_size);
     for(int i=0; i<ballots_size; i++)
     {
          cout << "\n---NEW BALLOT---\n\n";
          vector<set<int>> to_insert;
          to_insert.reserve(candnum);
          for(int j=1; j<=candnum; j++)
          {
	       set<int> equiv_prefs;
	       bool keep_going = true;
               while(keep_going)
               {
                    cout << "Enter name of candidate for choice #" << j << ": ";
                    getline(cin,temp);
		    if(temp[temp.length()-1]=='&')
		    {
			 temp = temp.substr(0,temp.length()-1);
			 j++;
		    }
		    else
			 keep_going = false;

		    auto it = find(candidates.begin(),candidates.end(),temp);
		    int x = it==candidates.end() ? -1 : it-candidates.begin();
		    if(x==-1 || equiv_prefs.count(x))
		    {
			 if(keep_going)
			      j--;

			 cout << "ERROR: invalid input" << endl;
			 keep_going = true;
		    }
		    else
			 equiv_prefs.insert(x);
               }
               to_insert.push_back(equiv_prefs);
          }

          ballots.push_back(to_insert);
     }

     //Input done.  Now process it.

     //Make Head2Head contests
     vector<Head2Head> contests;
     for(int i=0; i<candnum; i++)
          for(int j=i+1; j<candnum; j++)
               contests.push_back(Head2Head{i,j,0});

     //Process Head2Head contests
     for(auto& contest : contests)
          for(const auto& ballot : ballots)
          {
               const auto& winner_ = find_if(ballot.begin(),ballot.end(),
					     [&](const set<int>& p)
                                    {
                                         if(p.count(contest.first) && !p.count(contest.second))
                                              return true;
                                         else if(p.count(contest.second) && !p.count(contest.first))
                                              return true;
                                         return false;
                                    });
               if(winner_==ballot.end())
                    continue;
               const auto& winner = *winner_;
               if(winner.count(contest.first))
                    contest.score--;
               else
                    contest.score++;
          }

     //Shuffle head-to-heads, then stable sort
     random_shuffle(contests.begin(),contests.end());
     stable_sort(contests.begin(),contests.end(),
                 [](const Head2Head& left, const Head2Head& right)
                 {
                      if(abs(left.score) > abs(right.score))
                           return true;
                      return false;
                 });
     
     //Create graph nodes
     vector<GraphNode> graph;
     graph.reserve(candnum);
     for(int i=0; i<candnum; i++)
     {
          GraphNode node;
          node.id = i;
          graph.push_back(node);
     }

     //Process head-to-heads, in order
     for(const auto& contest : contests)
     {
          if(contest.score==0)
               break;
          
          int origin = contest.second, destination = contest.first;
          if(contest.score < 0)
          {
               origin = contest.first;
               destination = contest.second;
          }

          //Add edge
          graph[origin].edges.insert(destination);

          //Remove edge and add its opposite if edge created cycle
          if(is_reachable(graph,destination,origin))
          {
               graph[origin].edges.erase(destination);
               graph[destination].edges.insert(origin);
          }
     }

     //Generate ranking of winners
     int place = 1;
     while(true)
     {
          set<int> sources;
          for(int i=0; i<candnum; i++)
               if(graph[i].id==i)
                    sources.insert(i);

          if(!sources.size())
               break;

          for(const GraphNode& x : graph)
               for(int y : x.edges)
                    sources.erase(y);

          for(int x : sources)
               cout << "Place #" << place << ": " << candidates[x] << ", with " << graph[x].edges.size() << " direct wins.\n";

          //Remove winners
          for(int x : sources)
          {
               for(GraphNode& y : graph)
                    y.edges.erase(x);
               graph[x].id=-1;
               graph[x].edges.clear();
          }

          place++;
     }

     return 0;
}
