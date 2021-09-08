#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include<unistd.h>
#include<map>
#include "mylib.h"
#include<errno.h>
#include<arpa/inet.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <fstream>

#define MAXHOSTNAME 256
#define MAXCLIENTS 5

using namespace std;

cord *locations[MAXCLIENTS] = {NULL};
int clientNumber = 0;


int main()
{


   struct sockaddr_in socketInfo;
   char sysHost[MAXHOSTNAME+1];  // Hostname of this computer we are running on
   struct hostent *hPtr;
   int socketHandle;
   int portNumber = 8083;
   int opt = 1;
   int max_sd;  
   int sd;
   int activity;
   int new_socket;
   int addresslen;

   fd_set readfds;

   int client_socket[MAXCLIENTS + 1];

   for (int i = 0; i < MAXCLIENTS + 1; i++)
   {
      client_socket[i] = -1;
   }

   bzero(&socketInfo, sizeof(sockaddr_in));  // Clear structure memory

   // Get system information

   gethostname(sysHost, MAXHOSTNAME); // Get the name of this computer we are running on
   if((hPtr = gethostbyname(sysHost)) == NULL)
   {
      cerr << "System hostname misconfigured." << endl;
      exit(EXIT_FAILURE);
   }

   // create socket

   if((socketHandle = socket(AF_INET, SOCK_STREAM, 0)) == 0)
   {
      close(socketHandle);
      exit(EXIT_FAILURE);
   }
    cout << "created socket" << endl;


   // if (setsockopt(socketHandle,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt)) < 0)
   // {
   //    perror("setsocopt");
   //    exit(EXIT_FAILURE);
   // }

   // Load system information into socket data structures

   socketInfo.sin_family = AF_INET;
   socketInfo.sin_addr.s_addr = htonl(INADDR_ANY); // Use any address available to the system
   socketInfo.sin_port = htons(portNumber);      // Set port number

   // Bind the socket to a local socket address

   if( bind(socketHandle, (struct sockaddr *) &socketInfo, sizeof(socketInfo)) < 0)
   {
      close(socketHandle);
      perror("bind");
      exit(EXIT_FAILURE);
   }

   listen(socketHandle, MAXCLIENTS);

   cout << "Listening on port " << portNumber << endl;

   addresslen = sizeof(socketInfo); 

   while (true)
   {
      FD_ZERO(&readfds);

      FD_SET(socketHandle,&readfds);
      max_sd = socketHandle;

      // add socket to set
      for (int i = 0; i <= MAXCLIENTS; i++)
      {
         sd = client_socket[i];

         if (sd  > 0) FD_SET(sd,&readfds);

         if (sd > max_sd) max_sd = sd;
      }

      activity = select(max_sd+1,&readfds,NULL,NULL,NULL);
      
      if ((activity < 0) && (errno != EINTR)) perror ("Select error");

      // incoming connection
      if (FD_ISSET(socketHandle,&readfds))
      {
         if ((new_socket = accept(socketHandle,NULL,NULL)) < 0)
         {
            perror("accept");
            exit(EXIT_FAILURE);
         }

         cout << "New connection!!! ";

         // receive type of client
         int *type = new int;
         recv(new_socket,type,sizeof(int),0);

         if (*type == 1) // location client
         {
            if (clientNumber < MAXCLIENTS)
            {
               
               // send confirm to vehicle client
               bool *p = new bool;
               *p = true;
               send(new_socket,p,sizeof(bool),0);

               // delete pointer
               delete p;

               // add client to list
               for (int i = 0; i < MAXCLIENTS; i++)
               {
                  if (client_socket[i] == -1)
                  {
                     client_socket[i] = new_socket;
                  
                     cout << "Added new connection to list at " << i << "\n";
                     clientNumber++;
                     break;
                  }
               }

            }
            else
            {
               // send confirm to client
               bool *p = new bool;
               *p = false;
               send(new_socket,p,sizeof(bool),0);

               // delete pointer
               delete p;
            }
         }
         else
         {
            if (client_socket[MAXCLIENTS] == -1)
            {
               client_socket[MAXCLIENTS] = new_socket;
               cout << "An map client connected" << endl;

               // send confirm to client
               bool *p = new bool;
               *p = true;
               send(new_socket,p,sizeof(bool),0);
               // delete pointer
               delete p;
            }
            else
            {
               // send confirm to client
               bool *p = new bool;
               *p = false;
               send(new_socket,p,sizeof(bool),0);
               // delete pointer
               delete p;
            }
               
         }

      }
      
        // check client map
       /* if (client_socket[MAXCLIENTS] != -1)
        {
            if (FD_ISSET(client_socket[MAXCLIENTS],&readfds))
            {
                int * pointer = new int;
                int rc = recv(client_socket[MAXCLIENTS],pointer,4,0);
                if (rc == 0)
                {
                    cout << "Map client disconnected" << endl;
                    client_socket[MAXCLIENTS] = -1;
                }
            }
        } */

      for (int i = 0; i <= MAXCLIENTS; i++)
      {
         
         if (client_socket[i] == -1) continue;
         sd = client_socket[i];


         if (FD_ISSET(sd,&readfds))
         {
            if (i == MAXCLIENTS) // map client
            {
               bool *temp = new bool; 
               if(recv(sd,temp,sizeof(bool),0) == 0) // disconnected
               {
                  cout << "Map client disconnected" << endl;
                  client_socket[i] = -1;
               }
               continue;
            } 

            // handle and check client
            if (!handleClient(sd,locations[i],locations))
            {
               // handle client
               cout << "Client " << locations[i]->name << " disconected" << endl;
               client_socket[i] = -1;
               if(client_socket[MAXCLIENTS] == -1)
               {
                  delete locations[i];
                  locations[i] = NULL;
               } 
               clientNumber--;
            }

            if (client_socket[MAXCLIENTS] != -1)
            {
               if (locations[i] == NULL) continue;
                if (locations[i]->x == -1) continue;
               // send(client_socket[MAXCLIENTS],&clientNumber,sizeof(int),0);
               
               {
                    if (client_socket[i] != -1)
                    {
                       try
                       {
                          send(client_socket[MAXCLIENTS],locations[i],sizeof(cord),0);
                       }
                       catch(const int e)
                       {
                          
                          cout << "Map client disconnected" << endl;
                          client_socket[MAXCLIENTS] = -1;
                       }
                       
                        
                    }
                    else
                    {
                        locations[i]->x = -1.0;
                        locations[i]->y = -1.0;
                        try
                        {
                           send(client_socket[MAXCLIENTS],locations[i],sizeof(cord),0);
                        }
                        catch(const std::exception& e)
                        {
                           std::cerr << e.what() << '\n';
                           client_socket[MAXCLIENTS] = -1;
                        }
                        delete locations[i];
                        locations[i] = NULL;
                    }
               }
            }
         }
      }

     
      cout << "-----------------------------------------------" << endl;
   }
}
