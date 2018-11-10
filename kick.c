// Version May 04 2009, Revised by Jingtao Chen @Nov.11/2018, r.a.0.1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
struct Atom
{
	char name[40];
	int number;
};
#define atom struct Atom
struct Molecule
{
	char name[4];
	float coord[3];
};
#define molecule struct Molecule

float dist(int i, int j);
extern float atomicRadii(char *symbol);
int readInstructions(atom* atoms);
int find(char*);
int setupBox(float[3]);
int parse(char *instruction, atom *atoms);
void takeHeader(FILE* input);
void shrink();
int generateStructure();
int makeFile(int number, char[], int flag);
int fillMolecule();
int compar(const float* d1, const float* d2);
long search(FILE*, char* pattern);
int checkNormalTermination(char fileName[]);
int checkFinished(int i);	
int SaveCopy(const char * fn, const char * postfix);
char * com2log(const char * fn);

char header[4096];
char tail[4096];
int types;
int population;
float box[3];
int charge=0, mult=1;
int numberAtoms;
int checkParts();
int parts[50][50];
long position;
int iter=0;
FILE *com, *out;
atom atoms[100];
molecule mol[100];
int min=0;					//new fishished job
char directory[80];
int *files;
int main(int varc, char** var)
{
	int i=0, j=0;
	char fileName[20];
	char command[40]="g09 ";
	int structure=0;
	if (readInstructions(atoms))
		return 1;
	numberAtoms=fillMolecule();
	if ((box[0]<1)||(box[1]<1)||(box[2]<1))
		setupBox(box);
	printf("Box size is %.2f %.2f %.2f. %d files with random structures were generated\n",box[0],box[1],box[2], population);
	files=calloc(population,sizeof(int));
	srand(time(0));
	for (structure=0; structure < population; structure++)
	{
		makeFile(structure, fileName, 0);
		strcat(command,fileName);
		printf(" %s ",command);
		system(command);
		// return 0;
		if (checkNormalTermination(fileName))
		{
			SaveCopy(fileName, "EN1");
			SaveCopy(com2log(fileName), "EN1");
			makeFile(structure, fileName, 1);
			system(command);
		}
		if (checkNormalTermination(fileName)==1){
			SaveCopy(fileName, "EN2");
			SaveCopy(com2log(fileName), "EN2");
			system(command);
		}
		command[4]='\0';
	}
	return 0;

}

int readInstructions(atom *atoms)
{
	char* instruction;
	char dump[40];
	instruction=malloc(80*sizeof(char));
	FILE *ins;
	if (ins=fopen("INS","r")) {
		while(fgets(instruction,80,ins))
		{ 
			switch (instruction[0]) 
			{
				case ('p'): 
					sscanf(instruction,"%s %d",dump,&population); break;
				case ('c'): 
					sscanf(instruction,"%s %d %d",dump, &charge, &mult); break;
				case ('b'): 
					sscanf(instruction,"%s %f %f %f",dump,box,box+1,box+2); break;
				case ('a'): parse(instruction,atoms); break; 
				case ('h'): takeHeader(ins); fclose(ins); return 0; 
				case ('!'): case ('\n'): case(' '): break;
				default: 
								    printf("unknown instruction: %s\n",instruction);
			}
		}
		fclose(ins);	
		return 0;
	}
	else
		return 1;
}

int parse(char *instruction, atom *atoms)
{
	char symbol[40];
	int coeff=0;
	char *a;
	instruction=strchr(instruction,' ')+1;
	while(sscanf(instruction,"%s %d",symbol,&coeff)==2)
	{
		strcpy(atoms[types].name,symbol);
		atoms[types].number=coeff;
		types++;
		instruction=strchr(instruction,' ')+1;
		if (a=strchr(instruction,' '))
			instruction=a+1;
	}
	return 0;
}



int find(char* pattern)
{
	int lenghth=strlen(pattern);


}

int setupBox(float *box)
{
	float size=0;
	int i=0;
	for (i=0; i< types; i++)
	{
		size+=2*atoms[i].number*atomicRadii(atoms[i].name);
	}
	box[0]=size;
	box[1]=size;
	box[2]=size;
	return 0;
}


int makeFile(int number, char fileName[], int flag)
{
//	printf("\n STRUCTURE NUMBER %d \n",number);
	int i=0;
	char coord[40];					//line with atoms and coord
	char chargmult[8];				//line with charge and mult
	sprintf(fileName,"struct%d.com",number);
	com=fopen(fileName,"w");
	fprintf(com,"%%chk=struct%d.chk\n",number);
	
	// ==== REMOVE Chars from the tail of header ====
	char *p;
	for(p=header;*p;p++);  // MOVE to LAST
	while(p--){
		// printf("%d/", *p);
		if(*p > 32 ) break;
	}
	fwrite(header, sizeof(char), p-header+1, com);
	if (flag==1){
		fputs(" geom=check\n",com);
	}
	else{
		fputs("\n",com);
	}

	// ========= FUCK C89 ============
	

	sprintf(chargmult,"\nkick structure # %d\n\n%d %d\n",number,charge,mult);
	fputs(chargmult,com);
	if (flag==0)
	{
		generateStructure();
		for (i=0; i<numberAtoms; i++)
		{
			sprintf(coord,"%-2s %9.4f %9.4f %9.4f\n", mol[i].name, mol[i].coord[0], mol[i].coord[1], mol[i].coord[2]);
			fputs(coord,com);
		}
	}
	fputs("\n",com);
	fprintf(com, "%s", tail);
	fclose(com);
	return 0;

}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void takeHeader(FILE *ins)
{

	char * line = NULL;
    size_t len = 128;
	
	_Bool has_tail = false;
	ssize_t read;

	while((read = getline(&line, &len, ins)) != -1)
	{
		if(startsWith(">TAIL", line)){has_tail=true; break;}
		strcat(header, line);
	}

	if(has_tail){
		while((read = getline(&line, &len, ins)) != -1){
			strcat(tail, line);
		}
	}
	if (line)
        free(line);

}


int generateStructure()
{
	int i=0, k=0;
	for (i=0; i<numberAtoms;i++)
	{
		mol[i].coord[0]=box[0]*(float)rand()/RAND_MAX-box[0]/2;
		mol[i].coord[1]=box[1]*(float)rand()/RAND_MAX-box[1]/2;
		mol[i].coord[2]=box[2]*(float)rand()/RAND_MAX-box[2]/2;
	}
	float *distances=malloc(numberAtoms*(numberAtoms-1)*sizeof(float));
	while(checkParts(distances)!=1)
		shrink();
	return 0;
}

int fillMolecule()
{
	int i=0, j=0, k=0;
	for (i=0; i<types; i++)
		for (j=0; j<atoms[i].number; j++)
		{
			//			printf("%s\n",atoms[i].name);
			strncpy(mol[k].name,atoms[i].name,2);
			k++;
		}
	return k;
}


float dist(int i, int j)
{
	float x=mol[i].coord[0]-mol[j].coord[0];
	float y=mol[i].coord[1]-mol[j].coord[1];
	float z=mol[i].coord[2]-mol[j].coord[2];
	return sqrt(x*x+y*y+z*z);
}

int compar(const float* f1, const float* f2)
{
	if (*f1 > *f2)
		return 1;
	else 
		return -1;

}

int checkParts(distances)
{
	//	int **parts=calloc(numberAtoms*numberAtoms,sizeof(int));
	float distance=0;
	int numberParts=numberAtoms;
	int i=0, j=0, k=0, found=0;
	for (i=0; i<numberAtoms; i++)
		for (j=i+1; j<numberAtoms; j++)
		{
			distance=dist(i,j)/(atomicRadii(mol[i].name)+atomicRadii(mol[j].name));
			if (distance < 1.2)
			{
				parts[i][j]=1;
				parts[j][i]=1;
				//				printf("%d %d\n",i,j);
			}
			else 
			{
				parts[i][j]=0;
				parts[j][i]=0;
			}
		}
	for (i=0; i<numberAtoms; i++)
		parts[i][i]=1;

	for (i=0; i<numberAtoms; i++)
		if(parts[i][i]==1)
		{
			do
			{
				found=0;
				for (j=i+1; j<numberAtoms; j++)
				{
					if (parts[i][j]==1)
					{
						parts[i][j]=2;
						for (k=i+1; k<numberAtoms; k++)
							if ((parts[j][k]==1)&&(parts[i][k]==0))
							{
								parts[i][k]=1;
							}
						found=1;
						parts[i][i]++;
						parts[j][j]=0;
						numberParts--;
					}
				}
			}
			while (found);
		}
	return numberParts;
}


void shrink()
{
	int i=0, j=0;
	float coeff=1;
	float r=1;
	float center[3]={0,0,0};
	float center1[3]={0,0,0};
	for (i=0; i<numberAtoms; i++)
	{
		center[0]+=mol[i].coord[0];
		center[1]+=mol[i].coord[1];
		center[2]+=mol[i].coord[2];
	}
	center[0]/=numberAtoms;
	center[1]/=numberAtoms;
	center[2]/=numberAtoms;
	//	printf("center %f %f %f ",center[0],center[1],center[2]);
	for (i=0; i<numberAtoms; i++)
	{
		if (parts[i][i]==1)
		{
			r=sqrt(pow(mol[i].coord[0]-center[0],2)+pow(mol[i].coord[1]-center[1],2)+pow(mol[i].coord[2]-center[2],2));
			if (r > 0.2)
			{
				coeff=r/(r-0.2);
				//				printf(" coord %f %f %f, coeff %f ",mol[i].coord[0], mol[i].coord[1], mol[i].coord[2], coeff);
				mol[i].coord[0]=(mol[i].coord[0]-center[0])/coeff+center[0];
				mol[i].coord[1]=(mol[i].coord[1]-center[1])/coeff+center[1];
				mol[i].coord[2]=(mol[i].coord[2]-center[2])/coeff+center[2];
			}
		}
		if (parts[i][i]>1)
		{
			center1[0]=0;
			center1[1]=0;
			center1[2]=0;
			for (j=i; j<numberAtoms; j++)
				if (parts[i][j]!=0)
				{
					center1[0]+=mol[j].coord[0];
					center1[1]+=mol[j].coord[1];
					center1[2]+=mol[j].coord[2];
				}
			center1[0]/=parts[i][i];
			center1[1]/=parts[i][i];
			center1[2]/=parts[i][i];
			r=sqrt(pow(center1[0]-center[0],2)+pow(center1[1]-center[1],2)+pow(center1[2]-center[2],2));
			if (r > 0.2)
			{
				coeff=r/(r-0.2);

				for (j=i; j<numberAtoms; j++)
					if (parts[i][j]!=0)
					{
						mol[j].coord[0]+=(-center1[0]+(center1[0]-center[0])/coeff+center[0]);
						mol[j].coord[1]+=(-center1[1]+(center1[1]-center[1])/coeff+center[1]);
						mol[j].coord[2]+=(-center1[2]+(center1[2]-center[2])/coeff+center[2]);
					}
			}
		}
	}
	iter++;
}

int checkNormalTermination(char fileName[])
{
	char *dot;
	char outFileName[80];
	strcpy(outFileName,fileName);
	dot=strrchr(outFileName,'.');
	*(dot+1)='\0';
	strcat(outFileName,"log");
	out=fopen(outFileName,"r");
	if (out==NULL)
		return -1;
	fseek(out,-70,SEEK_END);
	if (search(out," Normal termination of Gaussian"))
	{
		fclose(out);
		return 0;
	}
	else
	{
		fclose(out);
		return 1;
	}
}


long int search(FILE* file, char* pattern)
{
	int i=0;
	int length=strlen(pattern);
	char current[length+1];
	if (fgets(current,length,file)!=NULL)
	{
		while (strcmp(current,pattern)!=0)
		{
			for (i = 0; i < length-1; i++)
				current[i]=current[i+1];
			current[length-1]=fgetc(file);
			current[length]='\0';
			if (current[length-1]==EOF)
				return 0;
		}
		printf(" Nashel %s ",pattern);
		return ftell(file);
	}
	return 0;
}


int checkFinished(int i)
{
	int a=0;
	char fileName[40];
	sprintf(fileName,"struct%d.log",i);
	if (files[i]==1)
		return 0;
	a=checkNormalTermination(fileName);
	if (a==-1)
	{
		files[i]=0;
		return 0;
	}
	else if (a==0)
	{
		files[i]=1;
		return 0;
	}
	else if (a==1)
	{
		files[i]++;
		return 1;
	}
	return 0;
}

int SaveCopy(const char * fn, const char * postfix){
	char tmp[4096];
	sprintf(tmp, "%s-%s", fn, postfix);
	
	FILE *input, *output;
    char ch;

    if((input = fopen(fn,"r")) == NULL)
    {
        printf("'%s' does not exist!\n", fn);
        return -1;
    }
    if((output = fopen(tmp,"w")) == NULL)
    {
        fclose(input);
        printf("Wrong output file name or Permission denied!\n");
        return -2;
    }
    ch = fgetc(input);
    while(!feof(input))
    {
        fputc(ch,output);
        ch = fgetc(input);
    }
    fclose(input);
    fclose(output);
    return 0;

}

char * com2log(const char * fn){
	char outFileName[80];
	strcpy(outFileName,fn);
	char* dot=strrchr(outFileName,'.');
	*(dot+1)='\0';
	strcat(outFileName,"log");
	return outFileName;
}