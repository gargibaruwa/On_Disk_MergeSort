#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct{
	//int file_num;
	int index;
	int end_index;
	int no_of_ele_read;
} file_info_s;

void basic_merge(char *inputfile,char *outputfile);
int create_basic_runs(char *inputfile,char *outputfile);
void basic_merge_sort(int no_of_ele, int run_file_no, char *inputfile);
void basic_merging(int j, char *inputfile, char *outputfile, int begin_file_no);	
void multistep_merge(char *inputfile,char *outputfile);
int create_super_runs(int j, char *inputfile);
void replacement_merge(char *inputfile,char *outputfile);
void heapify(int pos_root, int n);
void buildheap(int num_of_elements);
void h_sort(int num_of_elements);

int input_buf[1000];
int output_buf[1000];

void main(int argc, char*argv[])
{
	char *mergemethod = argv[1];
	char *inputfile = argv[2];
	char *outputfile = argv[3];

	if(strcmp(mergemethod,"--basic") == 0)
	{
		basic_merge(inputfile,outputfile);
	}
	else if(strcmp(mergemethod,"--multistep") == 0)
	{
		multistep_merge(inputfile,outputfile);
	}
	else if(strcmp(mergemethod,"--replacement") == 0)
	{
		replacement_merge(inputfile,outputfile);
	}
}

void basic_merge(char *inputfile,char *outputfile)
{
	struct timeval start_tm, end_tm, exec_tm;	
	gettimeofday( &start_tm, NULL );

	int no_of_run_files = create_basic_runs(inputfile,outputfile);
	int begin_file_no = 0;
	basic_merging(no_of_run_files,inputfile,outputfile, begin_file_no);

	gettimeofday( &end_tm, NULL );	

	long int exec_time = (end_tm.tv_sec*1000000 + end_tm.tv_usec) - (start_tm.tv_sec*1000000 + start_tm.tv_usec);
	exec_tm.tv_usec = exec_time % 1000000;
	exec_tm.tv_sec = exec_time / 1000000;
	printf( "Time: %ld.%06ld\n", exec_tm.tv_sec, exec_tm.tv_usec );	
}

int create_basic_runs(char *inputfile,char *outputfile)
{
	FILE *finp; 
	
	//check if input file exists
	finp = fopen( inputfile, "rb" );
	
	int key;
	int i = 0;
	int j = 0;
	while(fread( &key,sizeof(int),1,finp)==1)
	{
		if(i==1000)
		{				
			basic_merge_sort(i,j,inputfile);				
			j=j+1;
			i=0;
		}
		input_buf[i] = key;
		i++;		
	}
	basic_merge_sort(i,j,inputfile);	
	j = j+1;
	fclose(finp);	
	return j;
}

void basic_merge_sort(int no_of_ele, int run_file_no, char *inputfile)
{
	int i,j,n,x;
	n = no_of_ele;
	for(i=1; i<n; i++)
	{
		x = input_buf[i];
		j = i-1;
		while(j>=0 && input_buf[j] > x)
		{
			input_buf[j+1] = input_buf[j];
			j = j-1;
		}
		j = j+1;
		input_buf[j] = x;
	}
	
	char run_file_name[100];
	sprintf(run_file_name, "%s%c%03d", inputfile, '.', run_file_no);

	FILE *frun; 
	frun = fopen( run_file_name, "wb" ); 
	for(i=0;i<n;i++)		
	{
		fwrite(&input_buf[i],sizeof(int),1,frun); 
	}		
	fclose(frun);
}

void basic_merging(int j, char *inputfile, char *outputfile, int begin_file_no)
{
	int no_of_run_files = j;
	int keys_per_run = 1000/no_of_run_files;
	//printf("%d",keys_per_run);
	file_info_s file_info[no_of_run_files];

	int run_file_no = 0;
	FILE *fr;
	int temp;
	int i = 0;
	int count;
	char run_file_name[100];
	int inp_file_no = begin_file_no;
	
	while(run_file_no < no_of_run_files)
	{	
		sprintf(run_file_name, "%s%c%03d", inputfile, '.', inp_file_no);
		fr = fopen( run_file_name, "rb" );
		count = 0;
		while(fread( &temp,sizeof(int),1,fr)==1)
		{
			input_buf[i] = temp;
			i++;
			count++;	
			if(count==keys_per_run)
			{
				break;
			}
		}	
		
		fclose(fr);
		file_info[run_file_no].index = 0;
		file_info[run_file_no].end_index = keys_per_run;
		file_info[run_file_no].no_of_ele_read = keys_per_run;				
		run_file_no++;
		inp_file_no++;
	}
	
	int b,c,ct,min,min_file_no,out_pos,cnt,tmp;
	long offset;
	out_pos = 0;
	
	FILE *fout; 	
	/*if(( fout = fopen( outputfile, "r+b" )) == NULL ) 
	{
		fout = fopen( outputfile, "w+b" );
	}*/
	fout = fopen( outputfile, "w+b" );
	
	while(1)
	{
		c = 0;
		while(c<no_of_run_files)
		{
			if(file_info[c].index < file_info[c].end_index)
			{
				min = input_buf[(c*keys_per_run) + file_info[c].index];
				min_file_no = c; //file number
				break;
			}
			c++;
		}
			
		if(c == no_of_run_files)
		{
			break;
		}
		
		for(b=c+1; b<no_of_run_files; b++)
		{
			if(file_info[b].index < file_info[b].end_index)
			{
				if(min > input_buf[(b*keys_per_run) + file_info[b].index])
				{	
					min = input_buf[(b*keys_per_run) + file_info[b].index];
					min_file_no = b; //file number
				}
			}
		}
		
		//check if output buffer is full, if full, then write to sort.bin and write min to output_buf[0], else write min to output_buf[i]
		if(out_pos == 1000)
		{
			ct = 0;
			//append output_buf to sort.bin
			while(ct<1000)
			{
				fwrite( &output_buf[ct], sizeof( int ), 1, fout );
				ct++;
			}			
			out_pos = 0;
		}
		output_buf[out_pos] = min;
		out_pos++;
	
		//increment index of min
		file_info[min_file_no].index = file_info[min_file_no].index + 1;
	
		if(file_info[min_file_no].index == keys_per_run)
		{
			file_info[min_file_no].index = 0;
			file_info[min_file_no].end_index = 0;
	
			sprintf(run_file_name, "%s%c%03d", inputfile, '.', min_file_no+begin_file_no);
			fr = fopen( run_file_name, "rb" );
			offset = (file_info[min_file_no].no_of_ele_read)*(sizeof(int));
			fseek(fr,offset,SEEK_SET);	
			
			cnt = 0;
			while(cnt < keys_per_run)
			{
				if(fread( &tmp, sizeof(int), 1, fr)==1)
				{
					input_buf[min_file_no*keys_per_run + file_info[min_file_no].end_index] = tmp;
					file_info[min_file_no].end_index++;
					file_info[min_file_no].no_of_ele_read++;
					cnt++;
				}
				else
				{
					break;
				}			
			}
			fclose(fr);
		}
	}
	if(out_pos > 0)
	{
		ct = 0;
		//append output_buf to sort.bin
		while(ct < out_pos)
		{
			fwrite( &output_buf[ct], sizeof( int ), 1, fout );
			ct++;
		}			
	}
	fclose(fout);
}


void multistep_merge(char *inputfile,char *outputfile)
{
	struct timeval start_tm, end_tm, exec_tm;	
	gettimeofday( &start_tm, NULL );

	int no_of_run_files = create_basic_runs(inputfile,outputfile);
	int no_of_super_runs = create_super_runs(no_of_run_files,inputfile);
	int begin_file_no = 0;
	char super_run_file[100];
	strcpy(super_run_file,inputfile);
	strcat(super_run_file,".super");
	basic_merging(no_of_super_runs, super_run_file, outputfile, begin_file_no);

	gettimeofday( &end_tm, NULL );	

	long int exec_time = (end_tm.tv_sec*1000000 + end_tm.tv_usec) - (start_tm.tv_sec*1000000 + start_tm.tv_usec);
	exec_tm.tv_usec = exec_time % 1000000;
	exec_tm.tv_sec = exec_time / 1000000;
	printf( "Time: %ld.%06ld\n", exec_tm.tv_sec, exec_tm.tv_usec );	

}

int create_super_runs(int j,char *inputfile)
{
	int cnt_super_runs, no_of_run_files, i, begin_file_no, last_super_run;
	no_of_run_files = j;
	cnt_super_runs = no_of_run_files/15;
	last_super_run = no_of_run_files%15;
	char super_run_name[100];

	for(i=0; i<cnt_super_runs; i++)
	{
		sprintf(super_run_name, "%s%s%03d", inputfile, ".super.", i);
		begin_file_no = i*15;
		basic_merging(15, inputfile, super_run_name, begin_file_no);
	}
	if(last_super_run!=0)
	{
		sprintf(super_run_name, "%s%s%03d", inputfile, ".super.", i);
		begin_file_no = i*15;
		basic_merging(last_super_run, inputfile, super_run_name, begin_file_no);
		cnt_super_runs++;
	}
	return cnt_super_runs;
}

void replacement_merge(char *inputfile,char *outputfile)
{
	struct timeval start_tm, end_tm, exec_tm;	
	gettimeofday( &start_tm, NULL );


	FILE *finp; 
	
	//check if input file exists
	finp = fopen( inputfile, "rb" );
	
	int key, c, ct, out_pos;
	out_pos = 0;
	int i = 0;
	int run_file_no = 0;
	char run_file_name[100];
	int sec_heap_size = 0;
	int input_size = 250;
	int input_next = 750;
	int x;
	FILE *frun; 	

	int num_of_ele = 750;
	int flag=1;

	int sec_heap_pos, sec_heap_start;

	while(i<1000)
	{
		if(fread( &key,sizeof(int),1,finp)==1)
		{
			input_buf[i] = key;
			i++;
		}				
	}
	buildheap(num_of_ele);

	sprintf(run_file_name, "%s%c%03d", inputfile, '.', run_file_no);
	frun = fopen( run_file_name, "wb" );

	while(1)
	{
		if(num_of_ele==0 && sec_heap_size==0 && input_size==0)
		{
			break;
		}
		if(num_of_ele==0 || out_pos==1000)
		{
			ct = 0;
			while(ct<out_pos)
			{
				fwrite( &output_buf[ct], sizeof( int ), 1, frun );
				ct++;
			}
			if(num_of_ele==0)
			{
				fclose(frun);
				run_file_no++;
				sprintf(run_file_name, "%s%c%03d", inputfile, '.', run_file_no);
				frun = fopen( run_file_name, "wb" );		
				
				num_of_ele = sec_heap_size;
				sec_heap_size = 0;
				
				buildheap(num_of_ele);					
			}
			out_pos = 0;
		}
	
		output_buf[out_pos] = input_buf[0];
		out_pos++;	
	
		if(input_size!=0)
		{
			//input_start = num_of_ele + sec_heap_size;

			if(input_buf[0] <= input_buf[input_next])
			{
				input_buf[0]=input_buf[input_next];
			}
			else
			{
				input_buf[0]=input_buf[num_of_ele-1];
				input_buf[num_of_ele-1]=input_buf[input_next];
				num_of_ele--;	
				sec_heap_size++;
			}
			input_next++;
			input_size--;
		}
		else
		{
			input_buf[0]=input_buf[num_of_ele-1];
			for(x=num_of_ele-1; x<num_of_ele-1+sec_heap_size; x++)
			{
				input_buf[x] = input_buf[x+1];
			}
			num_of_ele--;
		}
		heapify(0,num_of_ele);
		if(input_size==0 && flag==1)
		{
			int input_pointer = 750;
			while(input_pointer<1000)
			{
				if(fread( &key,sizeof(int),1,finp)==1)
				{
					input_buf[input_pointer] = key;
					input_pointer++;
					input_size++;
				}	
				else
				{
					fclose(finp);
					flag = 0;
					break;
				}				
			}	
			input_next = 750;
		}		
	}
	if(out_pos > 0)
	{
		ct = 0;
		//append output_buf to sort.bin
		while(ct < out_pos)
		{
			fwrite( &output_buf[ct], sizeof( int ), 1, frun );
			ct++;
		}			
		fclose(frun);
		run_file_no++;	
	}	
	int begin_file_no = 0;
	basic_merging(run_file_no,inputfile,outputfile, begin_file_no);	

	gettimeofday( &end_tm, NULL );	

	long int exec_time = (end_tm.tv_sec*1000000 + end_tm.tv_usec) - (start_tm.tv_sec*1000000 + start_tm.tv_usec);
	exec_tm.tv_usec = exec_time % 1000000;
	exec_tm.tv_sec = exec_time / 1000000;
	printf( "Time: %ld.%06ld\n", exec_tm.tv_sec, exec_tm.tv_usec );	
}

void heapify(int pos_root, int n)
{
	if(n>0)
	{
		int left_child = (2*(pos_root+1))-1;
		int right_child = (2*(pos_root+1));
		//if pos_root is not a leaf
		if(left_child < n)//if left child exists
		{
			int pos_min_child;
			if(right_child < n && input_buf[left_child] > input_buf[right_child]) //if right child exists and is greater than left child
			{
				pos_min_child = right_child;
			}
			else
			{
				pos_min_child = left_child;
			}
			if(input_buf[pos_root] > input_buf[pos_min_child])
			{
				int temp = input_buf[pos_root];
				input_buf[pos_root] = input_buf[pos_min_child];
				input_buf[pos_min_child] = temp;
				heapify(pos_min_child,n);
			}
		}
	}
}
	
void buildheap(int num_of_elements)
{
	int start = (num_of_elements/2)-1;
	int i;
	for(i = start; i>=0; i--)
	{
		heapify(i,num_of_elements);
	}
}
