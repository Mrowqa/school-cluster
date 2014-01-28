#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
   int numprocs, rank, namelen;
   char processor_name[MPI_MAX_PROCESSOR_NAME];

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Get_processor_name(processor_name, &namelen);

   char privInfoBuff[256] = { 0 };
   sprintf(privInfoBuff, "UID:%i ; EUID:%i", getuid(), geteuid());

   if ( rank == 0 ) {
      printf( "[%02d/%02d %s]: I am the master (%s)\n", rank, numprocs, processor_name, privInfoBuff );
      // Tell the servants to do something
   } else {
      printf( "[%02d/%02d %s]: I am a servant (%s)\n", rank, numprocs, processor_name, privInfoBuff );
      // Wait for something to do
   }

   MPI_Finalize();
   return 0;
}
