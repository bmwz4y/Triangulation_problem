//Yahya Mubaideen	ID#20102171058
#include "mpi.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#define TAG1 101

int main(int argc, char* argv[])
{
	int rank, commSize, subSize, low, high, fileSize, characterRead, countNewLines = 0, tempInt;
	FILE *dataFile;
	int firstInt, secondInt, currentInt1, currentInt2, currentLow, currentHigh, iLoop, jLoop, myTriangles=0, totalTriangles=0;
	MPI_Request Request;
	clock_t beginTimer, endTimer;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);

	//begin timer to know how much it took to compute code
	if (rank == 0)
		beginTimer = clock();

	// if file exists and can be read
	if (access("data.txt", R_OK) != -1)
	{
		dataFile = fopen("data.txt","r");
//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 1);
//sleep(1);

		//if file has new lines equal or more than number of processors else make active only first rank
		//because then they will collide and make duplications of result
		if (rank == 0)
		{
			do
			{
				if (fgetc(dataFile) == '\n')
					countNewLines++;

			}while (!feof(dataFile) && countNewLines < commSize);

			//know file size
			fseek(dataFile, 0, SEEK_END);
			fileSize = ftell(dataFile);
		}

//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 2);
//sleep(1);
		//to make sure all have same before moving on
		MPI_Bcast(&countNewLines, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&fileSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

		if (countNewLines < commSize)
		{
			if (rank == 0)
			{
				low = 0;
				high = fileSize;
			}
			else //other ranks
			{
				low = 0;
				high = 0;
			}
		}
		else //new lines in file more than number of processors
		{
//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 3);
//sleep(1);
			//distribute between ranks if it can be done
			subSize = fileSize / commSize;

			//low and high offsets with respect to bytes in file
			low = rank * subSize;

			//going backwards to find last occurance of new line in previous portion of data which will be the start value for low
			//until it finds next difference in first integer in a line to make it real value of low
			//but not for first rank to avoid errors
			if (rank != 0)
			{
				fseek(dataFile, low, SEEK_SET);
				while (1)
				{
					if (fgetc(dataFile) == '\n')
					{
						//end while loop
						break;
					}
					fseek(dataFile, -2, SEEK_CUR);
				}

//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 4);
//sleep(1);
				//offset to next different first integer in the line
				tempInt = ftell(dataFile);
				fscanf(dataFile,"%d%*[^\n]", &firstInt);
				currentInt1 = firstInt;
				while(firstInt == currentInt1 && !feof(dataFile))
				{
					tempInt = ftell(dataFile);
					fscanf(dataFile,"%d%*[^\n]", &firstInt);
				}
				if (!feof(dataFile))
					low = tempInt;//to reverse last fscanf
				else
					low = fileSize;
				//send low of my portion to become high for previous portion
				MPI_Isend(&low, 1, MPI_INT, rank-1, TAG1, MPI_COMM_WORLD, &Request);
//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 5);
//sleep(1);
			}

			//recieving high for my portion of data from next rank
			//but not for last rank to avoid errors
			if (rank != commSize-1)
			{
				MPI_Recv(&high, 1, MPI_INT, rank+1, TAG1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else //last rank
			{
				high = fileSize;
//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 6);
//sleep(1);
			}
		}

		//each rank starts reading from his low offset
		fseek(dataFile, low, SEEK_SET);

		while(ftell(dataFile) < high)
		{
			currentLow = ftell(dataFile);
			tempInt = currentLow;
			fscanf(dataFile,"%d%*[^\n]", &firstInt);
			currentInt1 = firstInt;
			while(firstInt == currentInt1 && !feof(dataFile))
			{
				tempInt = ftell(dataFile);
				fscanf(dataFile,"%d%*[^\n]", &firstInt);
			}
			if (!feof(dataFile))
				currentHigh = tempInt;//to reverse last fscanf
			else
				currentHigh = fileSize;

//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 7);
//sleep(1);
			//now i have part of file that has same first integer
			//search if second int's second int is a second int for first int
			//in other words find if there is edge between any of the second ints in this part that has same first int
			for (iLoop = currentLow; iLoop < currentHigh; /* increment a line is inside */)
			{
				//to increment this loop correctly
				fseek(dataFile, iLoop, SEEK_SET);
				tempInt = iLoop;
				while(getc(dataFile) != '\n');
				iLoop = ftell(dataFile);
				fseek(dataFile, tempInt, SEEK_SET);

				fscanf(dataFile,"%*d%*[\t]%d%*[\n]", &secondInt);
				currentInt1 = secondInt;
				for (jLoop = ftell(dataFile); jLoop < currentHigh; /* increment a line is inside */)
				{
					//to increment this loop correctly
					tempInt = jLoop;
					while(getc(dataFile) != '\n');
					jLoop = ftell(dataFile);
					fseek(dataFile, tempInt, SEEK_SET);

					fscanf(dataFile,"%*d%*[\t]%d%*[\n]", &secondInt);
					currentInt2 = secondInt;

//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 8);
//sleep(1);
					//go to next first int
					//empty loop the file from currentHigh to first occurance of currentInt1
					fseek(dataFile, currentHigh, SEEK_SET);
					tempInt = currentHigh;
					fscanf(dataFile,"%d%*[^\n]", &firstInt);
					while(firstInt != currentInt1 && !feof(dataFile))
					{
						tempInt = ftell(dataFile);
						fscanf(dataFile,"%d%*[^\n]", &firstInt);
					}
					fseek(dataFile, tempInt, SEEK_CUR);
					while(firstInt == currentInt1 && !feof(dataFile))
					{
						fscanf(dataFile,"%d%*[\t]%d%*[\n]", &firstInt, &secondInt);
						if (firstInt == currentInt1 && secondInt == currentInt2)
							myTriangles++;
//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 9);
//sleep(1);
					}
				}
			}

		//to debug uncomment the lines below
		//printf("@proc[%d] first: %d, second: %d\n", rank, firstInt, secondInt);
		}

		//sum all rank triangles into global for all
		MPI_Reduce(&myTriangles, &totalTriangles, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		if (rank == 0)
		{
			printf("Total number of triangles: %d\n", totalTriangles);

			//end timer to know how much it took to compute code
			endTimer = clock();
			double timeElapsed = (double)(endTimer - beginTimer) / CLOCKS_PER_SEC;
			printf("Time it took to compute: %f seconds.\n", timeElapsed);
		}
		//to debug uncomment the lines below
		//printf("@proc[%d] has low=%d and high=%d\n", rank, low, high);

//to debug uncomment the lines below
//printf("@proc[%d] debug[%d]\n", rank, 10);
//sleep(1);
		fclose(dataFile);
	}
	else
	{
		if (rank == 0)
			printf("data.txt can't be found or isn't accessible.\n");
	}

	MPI_Finalize();
	return 0;
}
