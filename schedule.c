#include <stdio.h>
#include <stdlib.h>

int mtd;					//scheduling method
int num;					//process ����
int terminate =0;			//terminate�� process ����
int** info;					//process list ����
int CPU = -1;				//CPU�� �ִ� process�� ID
int* burst_time;
int* finish;
int* response;				
int* ready;					//ready queue
int request[1024];			//RR scheduling ready queue
int front_pt = 0;
int end_pt = 0;
int quantum = 2;
int burst = -1;

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

	finish = malloc(sizeof(int) * num);
	response = malloc(sizeof(int) * num);
	ready = malloc(sizeof(int) * num);
	burst_time = malloc(sizeof(int) * num);

	info = malloc(sizeof(int*) * num);

	for (int i = 0; i < num; i++)
	{
		info[i]= malloc(sizeof(int*) * 3);
	}
	for (int i = 0; i < idx; i++)
	{
		info[i / 3][i % 3] = buf[i];
		//printf("%d\n", info[i / 3][i % 3]);
	}
	for (int i = 0; i < num; i++)
	{
		burst_time[i] = info[i][2];
		ready[i] = 0;
		response[i] = -1;
	}
	for (int i = 0; i < 1024; i++)
	{
		request[i] = -1;
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
	burst++;
	int min_idx, min = 100, exist = 0;
	for (int i = 0; i < num; i++)
	{
		if (info[i][1]==tick)
		{
			printf("[tick: %02d ] New Process (ID: %d) newly joins to ready queue\n", tick, info[i][0]);
			ready[i] = 1;//ready queue ����
			request[end_pt] = i;
			end_pt++;
		}
	}
	switch (mtd)
	{
	case 1:
		//�������� ���μ����� ���� burst �ð� ������Ʈ
		if (CPU != -1) { burst_time[CPU]--; }
		if (CPU == -1 || !burst_time[CPU]) {	//context switching (CPU�� ����ų� ���μ����� ����)
			if (CPU != -1 && !burst_time[CPU]) {//terminate
				finish[CPU] = tick;
				terminate++;
				if (terminate == num) { break; } //��� ���μ��� terminate
			}
			if (request[front_pt] != -1)	//ready queue�� ���μ����� ����
			{
				CPU = request[front_pt];
				printf("[tick: %02d ] Dispatch to Process (ID: %d)\n", tick, info[CPU][0]);
				ready[CPU] = 0;						//ready queue���� ����
				response[CPU] = tick - info[CPU][1];//response time=���� tick - arrival time
				front_pt++;
				break;
			}
		}
		break;
	case 2:
		//�������� ���μ����� ���� burst �ð� ������Ʈ
		if (CPU != -1) { burst_time[CPU]--; }
		if (CPU != -1 && !burst_time[CPU]) {//terminate
			finish[CPU] = tick;
			terminate++;
			CPU = -1;
			if (terminate == num) { break; } //��� ���μ��� terminate
		}
		for (int i = 0; i < end_pt; i++) { //ready queue���� burst time�� ���� ª�� ���μ��� ã��
			if (burst_time[request[i]] < min && ready[request[i]] == 1) {
				min_idx = request[i];
				min = burst_time[min_idx];
				exist = 1;
			}
		}
		//context switching (CPU�� �����)
		if (exist && CPU == -1) {
			CPU = min_idx;
			ready[CPU] = 0;		//ready->running
			printf("[tick: %02d ] Dispatch to Process (ID: %d)\n", tick, info[CPU][0]);
			if (response[CPU] == -1) { response[CPU] = tick - info[CPU][1]; } //response time=���� tick - arrival time
		}
		break;
	case 3:
		//�������� ���μ����� ���� burst �ð� ������Ʈ
		if (CPU != -1) { burst_time[CPU]--; }
		if (CPU != -1 && !burst_time[CPU]) {//terminate
			finish[CPU] = tick;
			terminate++;
			CPU = -1;
			if (terminate == num) { break; } //��� ���μ��� terminate
		}


		for (int i = 0; i < end_pt; i++) { //ready queue���� ���� �ð��� ���� ª�� ���μ��� ã��
			if (burst_time[request[i]] < min && ready[request[i]] == 1) {
				min_idx = request[i];
				min = burst_time[min_idx];
				exist = 1;
			}
		}
		//context switching (CPU�� ����ų� ready queue�� ���μ����� CPU�� ���μ������� ���� �ð��� �� ª��)
		if (exist && (CPU == -1 || burst_time[min_idx] < burst_time[CPU])) {
			if (burst_time[min_idx] < burst_time[CPU])
			{
				ready[CPU] = 1;	//running->ready
			}
			CPU = min_idx;
			ready[CPU] = 0;		//ready->running
			printf("[tick: %02d ] Dispatch to Process (ID: %d)\n", tick, info[CPU][0]);
			if (response[CPU]==-1) { response[CPU] = tick - info[CPU][1]; } //response time=���� tick - arrival time
		}
		break;
	case 4:
		//�������� ���μ����� ���� burst �ð� ������Ʈ
		if (CPU != 0) { 
			burst_time[CPU - 1]--;
			quantum--;
		}
		if (quantum==0 || !CPU || !burst_time[CPU - 1])
		{
			if (!burst_time[CPU - 1])//terminate
			{
				finish[CPU - 1] = tick;
				terminate++;
			}
			if (request[front_pt] != 0)//context switching
			{
				int ID = request[front_pt];
				printf("[tick: %02d ] Dispatch to Process (ID: %d)\n", tick, ID);
				ready[ID - 1] = 0;
				if (CPU && burst_time[CPU - 1]) { //CPU�� �ִ� ���μ����� ready queue�� �̵�
					ready[CPU - 1] = 1;		
					request[end_pt] = CPU;
					end_pt++;
				} 
				if (response[ID - 1] == -1) {//CPU�� ó�� �Ҵ����
					response[ID - 1] = tick - info[ID - 1][1]; //response time = tick - arrival time
				}
				CPU = ID;
				quantum = 2;
				front_pt++;
			}
			//ready queue�� ������� ���� ���μ����� ������ ����
			else { quantum = 2; }
		}
		break;
	default:
		break;
	}
	
	if (num == terminate) {
		printf("[tick: %02d ] All processes are terminated.\n", tick);
		return 0;
	}
	return 1;
}

// fn: print_performance();
// desc: print system performance
void print_performance() {
	float turn = 0, wait = 0, res = 0;
	printf("===========================================================================================\n");
	printf("%5s%11s%9s%8s%20s%17s%16s\n", "PID", "arrival", "finish", "burst", "Turn around time", "Waiting time", "Response time");
	printf("===========================================================================================\n"); 
	
	for (int i = 0; i < num; i++)
	{
		printf("%4d%9d%9d%9d%14d%18d%16d\n", info[i][0], info[i][1], finish[i], info[i][2], finish[i] - info[i][1], finish[i]-info[i][1]-info[i][2], response[i]);
		turn += finish[i] - info[i][1];
		wait += finish[i] - info[i][1] - info[i][2];
		res += response[i];
	}
	printf("-------------------------------------------------------------------------------------------\n");
	printf("%9s%36.2f%18.2f%16.2f\n", "average:", turn / num, wait / num, res / num);
	printf("===========================================================================================\n");
}