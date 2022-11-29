#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"



typedef struct groupe{
   char nom[BUF_SIZE];
   char* membres[100];
   int nombreDeMembres;
}groupe;

groupe* LISTE_DE_GROUPES[100];

static void init(void)
{
printf("DEBUT INIT\r\n");

FILE* fichier;
fichier = fopen("Data/list_groups.txt","r");

char* buffer_group;
buffer_group = malloc(sizeof(char)* BUF_SIZE);
if(fichier!=NULL){
   printf("Fichier trouvé\r\n");
   char c;
   int compteurGroupes = 0;

   int compteurMembres;
   
   // crée le groupe
   c = fgetc(fichier);
   while(c!='+'){


      groupe* g = malloc(sizeof(groupe));
      // retour à 0 au retour à la ligne
      strcpy(buffer_group,"");
      compteurMembres = 0;
      while(c!='\n'){
         //c = fgetc(fichier);
         printf("%c",c);
         if(c==':'){
            // nom du groupe
            printf("\n|BUFFER NOM : %s |\n",buffer_group);
            strcpy(g->nom, buffer_group);
            compteurGroupes++;
            strcpy(buffer_group,"");
            c = fgetc(fichier);
         }
         if(c==';'){
            // nom de l'utilisateur a ajouter au groupe
            g->membres[compteurMembres] = malloc(sizeof(buffer_group));
            printf("\n|BUFFER MEMBRE : %s |\n",buffer_group);
            strcpy(g->membres[compteurMembres],buffer_group);
            compteurMembres++;
            strcpy(buffer_group,"");
         }
         else{
            //printf("\n|BUFFER CAT AVANT : %s |\n",buffer_group);
            strncat(buffer_group, &c, 1);
            //  buffer_group[strlen(buffer_group)] = c;
            //printf("\n|BUFFER CAT APRES : %s |\n",buffer_group);
         }
         c = fgetc(fichier);
      }
      //passe le retour a la ligne normalement
      c = fgetc(fichier);
      g->nombreDeMembres = compteurMembres;
      //ajoute le groupe à la liste de groupes
      LISTE_DE_GROUPES[compteurGroupes-1] = g;

      printf("| Nom Groupe fin : %s|\r\n",g->nom);
      printf("|1°membre: %s|\r\n", g->membres[0]);

      

   }

   afficherGroupes();
   printf("FIN INIT\r\n");
}

#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         if(read_client(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         Client c = { csock };
         strncpy(c.name, buffer, BUF_SIZE - 1);
         clients[actual] = c;
         actual++;
         printf("Client n°%d nom %s connecte\n", actual-1, clients[actual-1].name);
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
         {
            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
               printf("c:%i\r\n", c);
               printf("bufferSUPER:%s\r\n", buffer);
               /* client disconnected */
               if(strcmp(buffer, "quit") == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else
               {
                  if(strcmp(buffer,"list")==0){
                     int i;
                     for(i=0;i<2;i++){
                        // write_client(client.sock, LISTE_DE_GROUPES[i]);
                        // write_client(client.sock, "\n");
                     }
                  }else if(strcmp(buffer,"create")==0){
                     
                  }
                  
                  
                  else{
                     
                  send_message_to_all_clients(clients, client, actual, buffer, 0);
                  printf("Message de %s à tt le monde\n", client.name);
                  }
               
               }
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if(sender.sock != clients[i].sock)
      {
         if(from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
   }
}

static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;
   int i;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   } 
   
   //sizeof message - strlen(message) - 1

   // strncat(buffer, "", sizeof buffer - strlen(buffer) - 1);
   // printf("buffer:|%s|\r\n", buffer);
   // puts(buffer);

   // if(strcmp(buffer, "salut") == 0){
   //    printf("salutbg \r\n");
   // }
   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

static void afficherGroupes(){
   int i;
   for(i=0;i<8;i++){
      printf("Groupe : %s\r\n", LISTE_DE_GROUPES[i]->nom);
      int j;
      for(j=0; j<LISTE_DE_GROUPES[i]->nombreDeMembres; j++){
         printf("|%s|", LISTE_DE_GROUPES[i]->membres[j]);
      }
      printf("\n");
   }
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
