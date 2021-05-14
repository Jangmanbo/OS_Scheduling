#include <stdio.h>
#include <stdlib.h>

int mtd;
int num;
int** info;

// fn: read_proc_list
// desc: read process file list
// param
// 	file_name: process list name
void read_proc_list(const char* file_name) {
	char line[1024];
	int buf[1024], idx;
	FILE* pFile = fopen(file_name, "r");
	if (pFile != NULL && !feof(pFile)) {
		fgets(line, 1024, pFile);
		num = atoi(line);
		idx = 0;
		while (!feof(pFile)) {
			fgets(line, 1024, pFile);
			//printf("%s", line);
			char* ptr = strtok(line, " ");	// " " ���� ���ڸ� �������� ���ڿ��� �ڸ�, ������ ��ȯ
			while (ptr != NULL)				// �ڸ� ���ڿ��� ������ ���� ������ �ݺ�
			{
				//printf("%s\n", ptr);		// �ڸ� ���ڿ� ���
				if (ptr[0]!= '\n')
				{					
					buf[idx] = atoi(ptr);
					//printf("ptr:%s info:%d ", ptr, buf[idx]);
					idx++;
				}
				ptr = strtok(NULL, " ");	// ���� ���ڿ��� �߶� �����͸� ��ȯ
			}
		}
	}
	int row = idx / 3;
	info = malloc(sizeof(int*) * row);
	for (int i = 0; i < row; i++)
	{
		info[i]= malloc(sizeof(int*) * 3);
	}
	for (int i = 0; i < idx; i++)
	{
		info[i / 3][i % 3] = buf[i];
		//printf("%d\n", info[i / 3][i % 3]);
	}
}

// fn: set_schedule
// desc: set scheduling method
//
// param: method
// 	scheduling method
// 	1. FCFS (Nonpreemptive)
// 	2. Shortest Job First (Nonpreemptive)
// 	3. Shortest Remaining Time First (Prremptive)
//  4. Round Robin : Time Quantum = 2 (Prremptive)
//
// return none
void set_schedule(int method) {
	switch (method)
	{
	case 1:
		printf("%s", "Scheduling method: FCFS (Non-preemptive)\n");
		break;
	case 2:
		printf("%s", "Scheduling method: SJF (Non-preemptive)\n");
		break;
	case 3:
		printf("%s", "Scheduling method: SRTF (Preemptive)\n");
		break;
	case 4:
		printf("%s", "Scheduling method: RR (Preemptive)\n");
		break;
	default:
		break;
	}
	mtd = method;
}

// fn: do_schedule
// desc: scheduling function called every tick from main
// param
// 	tick: time tick beginning from 0
// return
//     0: when all process are terminated
//     1: otherwise
int do_schedule(int tick) {
	return 0;
}

// fn: print_performance();
// desc: print system performance
void print_performance() {}