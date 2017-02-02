#include <stdio.h>
#include <stdlib.h>

int func1();
int func2();
int func3();
int func4(char **);
int func5();

main()
{
	char mat[3][3],i,j;

	for(i = 0 ; i < 3 ; i++)
		for(j = 0 ; j < 3 ; j++)
		{
			mat[i][j] = i*10 + j;
		}

	printf(" Initialized data to: ");
	for(i = 0 ; i < 3 ; i++)
	{
		printf("\n");
		for(j = 0 ; j < 3 ; j++)
		{
			printf("%5.2d", mat[i][j]);
		}
	}
	printf("\n");

	func1(mat);
	func2(mat);
	func3(mat);
	func6((char **)mat);
	func5(mat);
}

 /* 
 Method #1 (No tricks, just an array with empty first dimension)
 ===============================================================
 You don't have to specify the first dimension! 
 */

int func1(char mat[][3])   
{
        register char i, j;

        printf(" Declare as matrix, explicitly specify second dimension: ");
        for(i = 0 ; i < 3 ; i++)
                {
                printf("\n");
                for(j = 0 ; j < 3 ; j++)
                {
	                printf("%5.2d", mat[i][j]);
                }
        }
        printf("\n");

        return;
}

 /*
 Method #2 (pointer to array, second dimension is explicitly specified)
 ======================================================================
 */

int func2(char (*mat)[3])
        {
        register char i, j;

        printf(" Declare as pointer to column, explicitly specify 2nd dim: ");
        for(i = 0 ; i < 3 ; i++)
                {
                printf("\n");
                for(j = 0 ; j < 3 ; j++)
                {
	                printf("%5.2d", mat[i][j]);
                }
        }
        printf("\n");

        return;
}

 /*
 Method #3 (Using a single pointer, the array is "flattened")
 ============================================================
 With this method you can create general-purpose routines.
 The dimensions doesn't appear in any declaration, so you 
 can add them to the formal argument list. 

 The manual array indexing will probably slow down execution.
 */

int func3(char *mat)	
        {
        register char i, j;

        printf(" Declare as single-pointer, manual offset computation: ");
        for(i = 0 ; i < 3 ; i++)
                {
                printf("\n");
                for(j = 0 ; j < 3 ; j++)
                {
	                printf("%5.2d", *(mat + 3*i + j));
                }
        }
        printf("\n");

        return;
}

 /*
 Method #4 (double pointer, using an auxiliary array of pointers)
 ================================================================
 With this method you can create general-purpose routines,
 if you allocate "index" at run-time. 

 Add the dimensions to the formal argument list.
 */

short func6(char **mat)
        {
        char    i, j, *index[3];

        for (i = 0 ; i < 3 ; i++)
                index[i] = (char *)mat + 3*i;

        printf(" Declare as double-pointer, use auxiliary pointer array: ");
        for(i = 0 ; i < 3 ; i++)
                {
                printf("\n");
                for(j = 0 ; j < 3 ; j++)
                {
	                printf("%5.2d", index[i][j]);
                }
        }
        printf("\n");

        return;
}

 /*
 Method #5 (single pointer, using an auxiliary array of pointers)
 ================================================================
 */

int func5(char *mat[3])
        {
        char i, j, *index[3];
        for (i = 0 ; i < 3 ; i++)
                index[i] = (char *)mat + 3*i;

        printf(" Declare as single-pointer, use auxiliary pointer array: ");
        for(i = 0 ; i < 3 ; i++)
                {
                printf("\n");
                for(j = 0 ; j < 3 ; j++)
                {
	                printf("%5.2d", index[i][j]);
                }
        }
        printf("\n");
        return;
}

