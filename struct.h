#include <iostream>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fstream>

#define MAXNAME 21
#define MAXCLIENTS 5

using namespace std;
struct cord
{
   double x;
   double y;
   char name[MAXNAME];
};

bool compareName(char name1[],char name2[])
{
    for (int i = 0; i < MAXNAME; i++)
    {
        if (name1[i] != name2[i]) return false;
        if (name1[i] == '\0')
        {
           return true;
        }
        
    }
    
}

bool handleClient(int &socketConnection, cord* &location, cord *locations[])
{
    cord *p = new cord;
    int rc = 0;
    rc = recv(socketConnection,p,sizeof(cord),0);
    if (rc == 0)
    {
        delete p;
        return false;
    }


    // check name new connection
    if (location == NULL)
    {
        bool *check = new bool;
        for (int i = 0; i < MAXCLIENTS; i++)
        {
            if (locations[i] == NULL) continue;
            if (compareName(p->name,locations[i]->name)) // duplicate name
            {
                *check = false;
                send(socketConnection,check,sizeof(bool),0);
                return true;
            } 
        }
        *check = true;
        send(socketConnection,check,sizeof(bool),0); // name validated
        location = new cord;
        location ->x = -1.0;
        delete check;
        return true;
    } 
    location->x = p->x;
    location->y = p->y;
    for (int i = 0; i < MAXNAME; i++)
    {
        location->name[i] = p->name[i];
    }

    

    cout << location->x << "," << location->y << endl;
    cout << location->name << endl;
    delete p;
    return true;
}
