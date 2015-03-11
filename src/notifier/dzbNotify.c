#include "dzbNotify.h"

int
main(int argc, char **argv)
{
    if(argc!=2){
        usage();
        exit(EXIT_FAILURE);
    }

    InitializeServer(&dazibaolist, argv[1]);

	pthread_attr_t attr;
	pthread_t tid1, tid2;

	if(0!=pthread_attr_init(&attr))
        errExit("pthread_attr_init");

    /* Watek sprawdza czy zaszly zmiany w plikach dazibao */
	if(0!=pthread_create(&tid1, &attr, &monitorDazibao, NULL))
        errExit("pthread_create");

    /* Watek dodaje nowych czytelnikow dazibao */
	if(0!=pthread_create(&tid2, &attr, &addReader, NULL))
        errExit("pthread_create");




	if(0!=pthread_attr_destroy(&attr))
        errExit("pthread_attr_destroy");

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);



    pthread_mutex_destroy(&listLock);
    pthread_mutex_destroy(&tableLock);

	return 0;

}

