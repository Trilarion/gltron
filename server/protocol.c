/********************************************************************************************
 *
 *  D�finition de la couche protocole. C'est ici qu'on fait appele aux fonctions de SDLNet.
 *  Auteur: Nicolas Deniaud ( nicolas.deniaud@wanadoo.fr ).
 *
 */

//Includes-----------------------------------------------------------------------------------

#include "gltron.h"
//Globals-------------------------------------------------------------------------------------


static SDLNet_SocketSet    socketSet = NULL;
static TCPsocket           tcpsock   = NULL; 

//********************************************************************************************

//Fonctions Generales

/** Net_init
 *  Initialise le r�seau. Ici on utilise SDL donc initialiation du r�seau
 */
void
Net_init()
{
  if( SDLNet_Init() < 0 )
    {
#ifdef __FRENCH__
      fprintf(stderr, "Impossible d'initialiser SDLNet: %s\n", SDLNet_GetError());
#else
      fprintf(stderr, "Can't init SDLNet %s\n", SDLNet_GetError());
#endif
      //exit(2);
    }
  //On pr�pare pour quand on va quitter
  atexit(Net_cleanup);
}

/** Net_cleanup
 *  Nettoie le r�seau.
 */
void
Net_cleanup()
{
  if( tcpsock != NULL )
    {
      SDLNet_TCP_Close(tcpsock);
      tcpsock = NULL;
    }
  if( socketSet != NULL )
    {
      SDLNet_FreeSocketSet(socketSet);
      socketSet = NULL;
    }
  
  SDLNet_Quit();
}

/** Net_connect
 *  �tablissement de la connection au serveur
 *  param�tres:
 *  server : adresse du serveur
 *  port   : port de connection
 *  renvoie le r�sultat
 */
int
Net_connect(char *server, int port)
{
  IPaddress      serverIP; //adresse IP

  SDLNet_ResolveHost(&serverIP, server, port);
  if( serverIP.host == INADDR_NONE )
    {
#ifdef __DEBUG__
      fprintf(stderr, "Impossible de trouver l'h�te %s.\n", server);
#endif
      return cantfindhost;
    } else {
      //Maintenant on se connecte
      tcpsock = SDLNet_TCP_Open(&serverIP);
      if( tcpsock == NULL )
	{
#ifdef __DEBUG__
	  fprintf(stderr, "La connection a �chou�.\n");
#endif
	  return cantconnect;	  
	} else {
#ifdef __DEBUG__
	  fprintf(stderr, "Connect�.\n");
#endif
	  return connected;
	}
    }
  return unkownError;
}

/** Net_deconnect
 *  D�connecte du r�seau
 */
void
Net_deconnect()
{
  if( tcpsock != NULL )
    {
      SDLNet_TCP_DelSocket(socketSet, tcpsock);
      SDLNet_TCP_Close(tcpsock);
      tcpsock = NULL;
    }
#ifdef __DEBUG__
  fprintf(stderr, "tcpsock d�connect�\n");
#endif
  isConnected=0;
}


/** Net_allocSocks
 *  Alloue le set de socket et ajout tcpsock.
 *  Ainsi on pourra surveiller le r�seau
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Net_allocSocks()
{
 //On alloue un set de socket
  socketSet = SDLNet_AllocSocketSet(2); //Le serveur + moi = 2 
  if( socketSet == NULL )
    {
#ifdef __DEBUG__
      fprintf(stderr, "Impossible d'alouer le set de socket: %s.\n", SDLNet_GetError());
#endif
      return cantallocsockset;
    }
  //On ajoute le socket
  SDLNet_TCP_AddSocket(socketSet, tcpsock);

  return noErr;
}

/** Net_checkSocks
 *  Regarde, l'activit� du r�seau
 *  et si c actif appel la fonction handle d�fini
 *  si pas actif renvoi 0, sinon 1
 */
int
Net_checkSocks()
{  
  SDLNet_CheckSockets(socketSet, 0);
  if( SDLNet_SocketReady(tcpsock) ) {
    return 1;
  }
  return 0;
}


/** Net_SockisValid
 *  V�rifie si tcpsock est bon.
 *  renvoi 1 si c ok, 0 sinon
 */
int
Net_SockisValid()
{
  return ( tcpsock != NULL );
}



//Sends: Fonction d'envoi de Donn�es...

/** Send_welcom
 *  Envoi le message de login
 *  param�tres:
 *  sock : socket de connection
 *  name : login de connection
 *  vers : version du client pour la comptatibilit� avec le serveur.
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Send_welcom( char *name, int vers)
{
  pWelcom   welcom;

  //Allocation de la m�moire
  welcom = ( pWelcom ) malloc( sizeof(tWelcom) );

  //Initialisation
  strcpy(welcom->name, name);
  welcom->vers = vers;
  
  //Envoi du packet
  if( ( SDLNet_TCP_Send(tcpsock, (char*)welcom, sizeof(tWelcom))) < sizeof(tWelcom))
    {
      //On lib�re la m�moire
      free(welcom);
      welcom = NULL;
      return connectClosed;
    }

  //On lib�re la m�moire
  free(welcom);
  welcom = NULL;

  return noErr;
}

/** Send_header
 *  Envoi une ent�te. Elle permet d'envoy� une information du serveur � un client
 *  ou l'inverse.
 *  param�tres:
 *  sock  : la socket en question
 *  type  : le type de message
 *  which : le slot d'origine
 *  len   : longeur du message qui suit l'ent�te
 *  time  : compteur de temps
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Send_header( int type, int which, int len, int time)
{
  pServRepHdr   serverRep;

  //Allocation de la m�moire
  serverRep = ( pServRepHdr ) malloc( sizeof(tServRepHdr) );

  //Initialisation
  serverRep->which = which;
  serverRep->type  = type;
  serverRep->len   = len;
  serverRep->time  = time;
  
  //Envoi du packet
  if( ( SDLNet_TCP_Send(tcpsock, (char*)serverRep, sizeof(tServRepHdr))) < sizeof(tServRepHdr))
    {
      //On lib�re la m�moire
      free(serverRep);
      serverRep = NULL;
      return connectClosed;
    }

  //On lib�re la m�moire
  free(serverRep);
  serverRep = NULL;

  return noErr;
}


/** Send_login
 *  R�ponse du serveur arp�s acceptation d'un login.
 *  il indique les informations de l'�tat du server
 *  param�tres:
 *  sock    : la socket en question
 *  state   : etat du serveur.
 *  nbUsers : nombre d'utilisateurs d�j� connect�s
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Send_login( int state, int nbUsers)
{
  pLogin loginRep;

  //Allocation de la m�moire
  loginRep = ( pLogin ) malloc( sizeof( tLogin ) );

  //Initialisation
  loginRep->state   = state;
  loginRep->nbUsers = nbUsers;

  //Envoi du packet
  if( ( SDLNet_TCP_Send(tcpsock, (char*)loginRep, sizeof(tLogin))) < sizeof(tLogin))
    {
      //On lib�re la m�moire
      free(loginRep);
      loginRep = NULL;
      return connectClosed;
    }

  //On lib�re la m�moire
  free(loginRep);
  loginRep = NULL;

  return noErr;
}

/** Send_who
 *  Envoi une info sur un utilsateur.
 *  param�tres:
 *  sock    : la socket en question
 *  which   : de qui on parle.
 *  pingIt  : le ping de cette utilisateur.
 *  color   : la couleur de l'utilisateur.
 *  isMaster: s'il est le maitre du jeu.
 *  name    : son nom.
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Send_who( int which, int pingIt, int color, int isMaster, char *name)
{
  pWho whoRep;

  //Allocation de la m�moire
  whoRep = ( pWho ) malloc( sizeof( tWho ) );

  //Initialisation
  whoRep->which    = which;
  whoRep->pingIt   = pingIt;
  whoRep->color    = color;
  whoRep->isMaster = isMaster;
  strcpy(whoRep->name, name);

  //Envoi du packet
  if( ( SDLNet_TCP_Send(tcpsock, (void *)whoRep, sizeof(tWho))) < sizeof(tWho))
    {
      free(whoRep);
      whoRep = NULL;
      return connectClosed;
    }

  //On lib�re la m�moire
  free(whoRep);
  whoRep = NULL;

  return noErr;
}

/** Send_chat
 *  Envoi un chat ( utilisable en pre-game ).
 *  param�tres:
 *  sock    : la socket en question
 *  text    : le texte du chat
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Send_chat( char *text)
{
  int len = strlen(text);

  //Envoi du packet
  if( ( SDLNet_TCP_Send(tcpsock, (void *)text, len)) < len)
    {
      return connectClosed;
    }

  return noErr;
}


/** Send_Buff
 *  Envoi len octets de buff sur le r�seau.
 *  param�tres:
 *  buff  : la chaine r�ponse
 */
int
Send_Buff(char *buff)
{
  int len = strlen(buff);

  if((SDLNet_TCP_Send(tcpsock, (char *)buff, len)) < len )
    {
      return connectClosed;
    }
  return noErr;
}

/** Send_gameEvent
 *  Envoi une info du jeu.
 *  param�tes:
 *  e : c un event de Game
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Send_gameEvent( GameEvent* e)
{
  int len = sizeof(GameEvent);

  if( ( SDLNet_TCP_Send(tcpsock, (void *)e, len)) < len)
    {
      return connectClosed;
    }
  
   return noErr;
}

/** Send_chgeState
 *  Envoi un changement d'�tat du serveur
 */
int
Send_chgeState(int newState)
{
  int len    = sizeof(int);
  int *state = (int *)malloc(len);

  *state=newState;

  fprintf(stderr, "change pour %d ( %d )\n", *state, len);

  if( ( SDLNet_TCP_Send(tcpsock, (void *)state, len)) < len)
    {
      return connectClosed;
    }
  fprintf(stderr, "Envoy�\n");
  free(state);
  return noErr;
}


/** Recvs : R�ception **/


/** Recv_header
 *  Recoit une ent�te.
 *  param�tres:
 *  sock  : la socket en question
 *  type  : le type de message
 *  which : le slot d'origine
 *  len   : longeur du message qui suit l'ent�te
 *  time  : compteur de temps
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Recv_header( int *type, int *which, int *len, int *time)
{
  int rLen;
  pServRepHdr   serverRep;

  //Allocation de la m�moire
  serverRep = ( pServRepHdr ) malloc( sizeof(tServRepHdr) );


  rLen = SDLNet_TCP_Recv(tcpsock, (void *)serverRep, sizeof(tServRepHdr));
  if( rLen <= 0 )
    {
      free(serverRep);
      serverRep=NULL;
      return connectLost;
    }
  //Initialisation
  *which = serverRep->which;
  *type  = serverRep->type;
  *len   = serverRep->len;
  *time  = serverRep->time;  


  free(serverRep);
  serverRep=NULL;
  return noErr;
}


/** Recv_login
 *  Recoit les infos de login.
 *  param�tres:
 *  sock    : la socket en question
 *  state   : etat du server
 *  nbUsers : nombre d'utilisateurs d�h� connect�
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Recv_login( int *state, int *nbUsers)
{
  int      rLen;
  pLogin   loginRep;

  //Allocation de la m�moire
  loginRep = ( pLogin ) malloc( sizeof(tLogin) );


  rLen = SDLNet_TCP_Recv(tcpsock, (void *)loginRep, sizeof(tLogin));
  if( rLen <= 0 )
    {
      free(loginRep);
      loginRep=NULL;
      return connectLost;
    }
  //Initialisation
  *state    = loginRep->state;
  *nbUsers  = loginRep->nbUsers;


  free(loginRep);
  loginRep=NULL;
  return noErr;
}


/** Recv_who
 *  Recoit les infos d'un utilisateur.
 *  param�tres:
 *  sock    : la socket en question
 *  slots   : tableaux des solts
 *  renvoie : which si pas d'erreur, le code d'erreur sinon.
 */
int
Recv_who( pslots slots) 
{
  int      rLen;
  pWho     whoRep;
  int      which;
  

  //Allocation de la m�moire
  whoRep = ( pWho ) malloc( sizeof(tWho) );
  
  rLen = SDLNet_TCP_Recv(tcpsock, (void *)whoRep, sizeof(tWho));
  if( rLen <= 0 )
    {
      free(whoRep);
      whoRep=NULL;
      return connectLost;
    }
  //Initialisation
  which                 = whoRep->which;
  slots[which].color    = whoRep->color;
  slots[which].pingIt   = whoRep->pingIt;
  strcpy(slots[which].name,whoRep->name);
  slots[which].active   = 1;
  slots[which].isMaster = whoRep->isMaster;
  free(whoRep);
  whoRep=NULL;
  return which;
}


/** Recv_chat
 *  Recoit du texte de chat. Alloue la m�moire
 *  param�tres:
 *  sock    : la socket en question
 *  len     : longueur du texte � lire
 *  text    : le texte
 *  renvoie : noErr si pas d'erreur, le code d'erreur sinon.
 */
int
Recv_chat( int len, char *text  )
{
  int rLen;

  //Allocation de la m�moire
  //text = ( char * ) malloc( len );

  rLen = SDLNet_TCP_Recv(tcpsock, (char *)text, len);
  if( rLen <= len )
    {
      return connectLost;
    }
  return noErr;
}

/** Recv_Buff
 *  Lit len octets et met dans un char * du r�seau.
 *  param�tres:
 *  buff  : la chaine r�ponse
 *  len   : longueur � lire
 */
void
Recv_Buff(char *buff, int len)
{
  // !!! !!! Ici v�rifier la m�moire....

  SDLNet_TCP_Recv(tcpsock, (char *)buff, len);
  buff[len] = '\0';
}


/** Recv_gameEvent
 *  Recoit une info du jeu.
 *  renvoie : L'event sinon NULL
 */
GameEvent*
Recv_gameEvent()
{
  int rLen;
  GameEvent *e= (GameEvent *) malloc(sizeof(GameEvent));
  
  rLen = SDLNet_TCP_Recv(tcpsock, (void *)e, sizeof(GameEvent));
  if( rLen <= 0 )
    {
      free(e);
      return NULL;
    }

   return e;
}

/** Recv_chgeState
 *  Recoit un changement d'�tat du serveur
 */
int
Recv_chgeState(int *newState)
{
  int rLen;

  rLen = SDLNet_TCP_Recv(tcpsock, (void *)newState, sizeof(int));
  if( rLen <= sizeof(int) )
    {
      return connectLost;
    }
  return noErr;
}

int
Recv_netGameSettings(netGameSettings ngS)
{
  int rLen;

  rLen = SDLNet_TCP_Recv(tcpsock, (void *)ngS, sizeof(tnetGameSettings));
  if( rLen <= sizeof(tnetGameSettings) )
    {
      return connectLost;
    }
  return noErr;
}

/** Creates */
void
defaultNetGameSettings(netGameSettings ngS)
{
  ngS->grid.width=640;
  ngS->grid.height=480;
  ngS->rule.eraseCrashed=1;
  ngS->rule.speed=8.50;
  ngS->timeR=0;
  ngS->nbGames=5;
}

netGameSettings
createNetGameSettings()
{
  netGameSettings ngS = ( netGameSettings ) malloc(sizeof(tnetGameSettings));
  defaultNetGameSettings(ngS);
  return ngS;
}

cnetEventList
createNetEventCell( GameEvent *e )
{
  cnetEventList cell= ( cnetEventList ) malloc(sizeof(tcnetEventList));
  cell->next = NULL;
  cell->event = *e;
  return cell;
}
netEventList
createNetEventList( void )
{
  netEventList nl = ( netEventList ) malloc(sizeof(tnetEventList));
  nl->head = NULL;
  return nl;
}

void
addNetEvent( GameEvent *e)
{
  cnetEventList cell;

  cell = neteventlist->head;
  if( cell == NULL )
    {
      neteventlist->head = createNetEventCell(e);
      return;
    }
  while( cell->next )
    { cell = cell->next; }
  cell->next = createNetEventCell(e);  
}

GameEvent *
getNetEvent()
{
  cnetEventList cell;
  GameEvent *e= ( GameEvent *) malloc(sizeof(GameEvent));

  if( neteventlist->head == NULL )
    return NULL;

  cell = neteventlist->head;

  neteventlist->head = cell->next;
  *e= cell->event;
  free(cell);
  return e;
}
/** Prints */


void
print_serverState(int serverState)
{
  switch( serverState )
    {
      case waitState: //Etat d'attend: personne sur le server...
#ifdef __FRENCH__
	fprintf(stderr, "Serveur en Etat d'attente\n");
#else
	fprintf(stderr, "Waiting state\n");
#endif
	break;
      case preGameState:
#ifdef __FRENCH__
	fprintf(stderr, "Server en Etat de pre-Game\n");
#else
	fprintf(stderr, "preGame state\n");
#endif
	break;
      case gameState:
#ifdef __FRENCH__
	fprintf(stderr, "Server en Etat de partie lanc�e\n");
#else
	fprintf(stderr, "Game start\n");
#endif
	break;
    default:
#ifdef __FRENCH__
      fprintf(stderr, "Etat du serveur inconnu!!!!\n");
#else
      fprintf(stderr, "Unknown Server State\n");
#endif    
      break;
    }
}

void
printNetGameSettings ( netGameSettings ngS )
{
#ifdef __FRENCH__
  fprintf(stderr, "R�glage de la partie:\ntaille de la grille:%dx%d\nEfface apr�s crash:%d\nVitesse:%f\nTemps d'une partie:%d\nNbre de jeux dans une partie:%d\n",
#else
  fprintf(stderr, "Game Settings:\nGrid Size:%dx%d\nErase Crashed:%d\nSpeed:%f\nTime in a Game:%d\nHow many games before coming back to Pre-game:%d\n",
#endif
  ngS->grid.width,
  ngS->grid.height,
  ngS->rule.eraseCrashed,
  ngS->rule.speed,
  ngS->timeR,
  ngS->nbGames);
}

