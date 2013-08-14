/*=========================================================================
  Program:   Memory load and dump (Prog HW 1)  ==   Class: EECE 6278
  Author:    Omoju Miller					   ==   Language:  C		
 =========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define OPCODE_MAX 100 
#define MEM_MAX 512

int load(FILE *fpt);
void dump(void); 
int opcode_index, mem[MEM_MAX], opcode[OPCODE_MAX], num_of_blocks[OPCODE_MAX], ic, i;

main()
{
	FILE *fpt;
	int ic;
	/* open the memory file for reading */
	if((fpt = fopen("memory_file.dat", "r")) == NULL)
		printf("\nERROR - Cannot open the designated file\n");
	else
	{   ic = load(fpt);
		dump();
	}
	fclose(fpt);
}

/* Load the memory of a simple computer */
int load(FILE *fpt)
{   /* local variable declaration and initialization */ 
	char c, address[20];
	int x, flag, i, mem_index, blocks;
	opcode_index = 0, mem_index = 0, flag = 0, blocks = 0;
	do
	{   i = 0;
		do
		{/* Read a character at at time from the file stream */
			c = getc(fpt); address[i] = c; /* stores the chars */ i++;
			if(flag && c == ' ') /* the case where there is a block like '  2343' */
			{  i = i - 1; c = getc(fpt); address[i] = c; i++; }
			flag = 0;
		}while (c != ' ' && c != '\n' && c != EOF);
		i = i - 1; address[i] = '\0'; x = strlen(address);
		if(x == 4) /* if 4 mem address */
		{   mem[mem_index] = atoi(address); mem_index++;
			num_of_blocks[opcode_index-1] = ++blocks; flag = 1;
		}
		else /* else and opcode */
		{   opcode[opcode_index] = atoi(address);
			opcode_index++; blocks = 0; flag = 1;
		}
	}while(opcode[opcode_index-1]!= opcode[0] || opcode_index-1 == 0);
	return opcode[opcode_index-1];
}

/* Dump - Outputs the content of the memory */
void dump(void)
{   int i, k, index, mem_index; mem_index = 0;
	for(i = 0; i < opcode_index-1; i++)
	{   printf("%o ", opcode[i]);
		for(index = 0; index < num_of_blocks[i]; index++)
		{   if(index == 3)
			{ printf("%o  ", mem[mem_index]); mem_index++;}
			else if(mem[mem_index] == 0)
			{ printf("%s ", "0000"); mem_index++; }
			else
			{ printf("%o ", mem[mem_index]); mem_index++; }	
		}
		if((8 - num_of_blocks[i]))
		{   for(k = 0; k < (8 - num_of_blocks[i]); k++)
			{
				if( k == ((8 - num_of_blocks[i])%4)) { printf("%s"," "); }
				printf("%s ", "0000");
			}
		} printf("\n");
	} printf("$\n");
}

