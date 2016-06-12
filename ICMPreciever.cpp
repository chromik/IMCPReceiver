#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>

#define BUFSIZE 1024

using namespace std;

int main(int argc, char *argv[])
{
  hostent *host; 
  iphdr *ip;
  icmphdr *icmp;
  int sock;
  unsigned int count;
  sockaddr_in sender;
  char buffer[BUFSIZE];
  int bufferLenght;
  socklen_t lenght;
  char *stringIP, *unknown = "?";

  if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
  {
    perror("Inicializace soketù");
    return -1;
  }
  while(true)
  {
    lenght = sizeof(sockaddr_in);
    if ((bufferLenght = recvfrom(sock, buffer, BUFSIZE, 0, (sockaddr *)&sender, &lenght)) == -1)
    {
      printf("Pøíjem dat: %u\n", 0);
      close(sock);
      return -1;
    }
    ip = (iphdr*)buffer;
    icmp = (icmphdr *) (buffer + ip->ihl * 4);
    stringIP = strdup(inet_ntoa(sender.sin_addr));
    host = gethostbyaddr((char *)&sender.sin_addr, 4, AF_INET);
    printf("\nPrisel ICMP paket velikosti %d bytu\nOdesilatel: %s (%s)\nTTL: %d\nTyp ICMP: %d\nKod ICMP: %d\n", bufferLenght, stringIP, (host == NULL? unknown : host->h_name), (int)ip->ttl, icmp->type, icmp->code);
    free(stringIP);
    switch (icmp->type)
    {
      case ICMP_ECHOREPLY: 
          printf("ECHO odpoved\n");
          if (icmp->code != 0)
          {
             printf("Ale nejaká divna!\n");
          }
          else
          {
             printf("Identifikator odpovedi: %d\nPoøadove cislo odpovedi: %d\n", icmp->un.echo.id, icmp->un.echo.sequence);
          }
          break;
      case ICMP_ECHO:
          printf("ECHO zadost\n");
          if (icmp->code != 0)
          {
             printf("Ale nejaká divna!\n");
          }
          else
          {
             printf("Identifikator zadosti: %d\nPoradové cislo zadosti: %d\n", icmp->un.echo.id, icmp->un.echo.sequence);
          }
          break;
      case ICMP_TIME_EXCEEDED:
          printf("Byl zahozen IP paket odeslany z tohoto pocitace.");
          if (icmp->code != ICMP_EXC_TTL)
          {
             printf("Kod paketu je: %d - v dokumentaci je napsano, co znamena\n", icmp->code);      
          }
          else
          {
             printf("Z tohoto pocitace byl odeslán IP paket, jehoz TTL na ceste kleslo na 0.\n");
          }
          break;
      case ICMP_DEST_UNREACH:
          printf("IP datagram, který byl odeslan z tohoto pocitace nelze dorucit.\n");
          if (icmp->code != ICMP_PORT_UNREACH)
          {
             printf("Kod paketu je: %d - v dokumentaci je napsano, co znamena\n", icmp->code);
          }
          else
          {
             printf("Nedosazitelny port\n");
          }
          break;
    }
  }
  close(sock);
  return 0;
}

