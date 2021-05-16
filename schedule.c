#include <stdio.h>
#include <stdlib.h>

int mtd;					//scheduling method
int num;					//process 개수
int terminate =0;			//terminate된 process 개수
int CPU = -1;				//CPU에 있는 process의 ID
int** info;					//process list 정보
int* burst;					//남은 시간
int* finish;				//finish time
int* response;				//response time
int* ready_exist;			//ready queue에 있는지 없는지
int ready[1024];			//ready queue
int front_pt = 0;			//ready queue pointer
int end_pt = 0;				//ready queue pointer
int quantum = 2;			//default:2

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
			char* ptr = strtok(line, " ");	// " " 공백 문자를 기준으로 문자열을 자름, 포인터 반환
			while (ptr != NULL)				// 자른 문자열이 나오지 않을 때까지 반복
			{
				if (ptr[0]!= '\n')
				{					
					buf[idx] = atoi(ptr);
					idx++;
				}
				ptr = strtok(NULL, " ");	// 다음 문자열을 잘라서 포인터를 반환
			}
		}
	}

	//동적할당
	finish = malloc(sizeof(int) * num);
	response = malloc(sizeof(int) * num);
	ready_exist = malloc(sizeof(int) * num);
	burst = malloc(sizeof(int) * num);

	info = malloc(sizeof(int*) * num);
	for (int i = 0; i < num; i++) { info[i] = malloc(sizeof(int*) * 3); }

	//process list 정보 저장
	for (int i = 0; i < idx; i++) { info[i / 3][i % 3] = buf[i]; }
	for (int i = 0; i < num; i++)
	{
		burst[i] = info[i][2];
		ready_exist[i] = 0;
		response[i] = -1;
	}

	//ready queue 초기화
	for (int i = 0; i < 1024; i++) { ready[i] = -1; }
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
	int min_idx, min = 100, exist = 0;
	for (int i = 0; i < num; i++)
	{
		if (info[i][1]==tick)
		{
			printf("[tick: %02d ] New Process (ID: %d) newly joins to ready queue\n", tick, info[i][0]);
			ready_exist[i] = 1;//ready queue 들어옴
			ready[end_pt] = i;
			end_pt++;
		}
	}
	switch (mtd)
	{
	case 1:
		//실행중인 프로세스의 남은 시간 업데이트
		if (CPU != -1) { burst[CPU]--; }
		if (CPU == -1 || !burst[CPU]) {	//context switching (CPU가 비었거나 프로세스가 끝남)
			if (CPU != -1 && !burst[CPU]) {//terminate
				finish[CPU] = tick;
				terminate++;
				if (terminate == num) { break; } //모든 프로세스 terminate
			}
			if (ready[front_pt] != -1)	//ready queue에 프로세스가 있음
			{
				CPU = ready[front_pt];
				printf("[tick: %02d ] Dispatch to Process (ID: %d)\n", tick, info[CPU][0]);
				ready_exist[CPU] = 0;						//ready queue에서 나옴
				response[CPU] = tick - info[CPU][1];//response time=현재 tick - arrival time
				front_pt++;
				break;
			}
		}
		break;
	case 2:
		//실행중인 프로세스의 남은 시간 업데이트
		if (CPU != -1) { burst[CPU]--; }
		if (CPU != -1 && !burst[CPU]) {//terminate
			finish[CPU] = tick;
			terminate++;
			CPU = -1;
			if (terminate == num) { break; } //모든 프로세스 terminate
		}
		for (int i = 0; i < end_pt; i++) { //ready queue에서 burst time이 가장 짧은 프로세스 찾기
			if (burst[ready[i]] < min && ready_exist[ready[i]] == 1) {
				min_idx = ready[i];
				min = burst[min_idx];
				exist = 1;
			}
		}
		//context switching (CPU가 비었음)
		if (exist && CPU == -1) {
			CPU = min_idx;
			ready_exist[CPU] = 0;		//ready->running
			printf("[tick: %02d ] Dispatch to Process (ID: %d)\n", tick, info[CPU][0]);
			if (response[CPU] == -1) { response[CPU] = tick - info[CPU][1]; } //response time=현재 tick - arrival time
		}
		break;
	case 3:
		//실행중인 프로세스의 남은 시간 업데이트
		if (CPU != -1) { burst[CPU]--; }
		if (CPU != -1 && !burst[CPU]) {//terminate
			finish[CPU] = tick;
			terminate++;
			CPU = -1;
			if (terminate == num) { break; } //모든 프로세스 terminate
		}
		for (int i = 0; i < end_pt; i++) { //ready queue에서 남은 시간이 가장 짧은 프로세스 찾기
			if (burst[ready[i]] < min && ready_exist[ready[i]] == 1) {
				min_idx = ready[i];
				min = burst[min_idx];
				exist = 1;
			}
		}
		//context switching (CPU가 비었거나 ready queue의 프로세스가 CPU의 프로세스보다 남은 시간이 더 짧음)
		if (exist && (CPU == -1 || burst[min_idx] < burst[CPU])) {
			if (burst[min_idx] < burst[CPU]) { ready_exist[CPU] = 1; }//running->ready
			CPU = min_idx;
			ready_exist[CPU] = 0;		//ready->running
			printf("[tick: %02d ] Dispatch to Process (ID: %d)\n", tick, info[CPU][0]);
			if (response[CPU]==-1) { response[CPU] = tick - info[CPU][1]; } //response time=현재 tick - arrival time
		}
		break;
	case 4:
		//실행중인 프로세스의 남은 시간, quantum 업데이트
		if (CPU != -1) { 
			burst[CPU]--;
			quantum--;
		}
		if (CPU != -1 && !burst[CPU]) {//terminate
			finish[CPU] = tick;
			terminate++;
			CPU = -1;
			if (terminate == num) { break; } //모든 프로세스 terminate
		}
		if ((CPU == -1 || !quantum) && ready[front_pt] != -1) {//context switching(CPU가 비었거나 quantum 지남)
			if (CPU != -1 && burst[CPU]) {//CPU의 프로세스가 더 실행되어야 함(running->ready)
				ready_exist[CPU - 1] = 1;
				ready[end_pt] = CPU;
				end_pt++;
			}
			CPU = ready[front_pt];
			ready_exist[CPU] = 0;		//ready->running
			front_pt++;
			quantum = 2;
			if (response[CPU] == -1) { response[CPU] = tick - info[CPU][1]; } //response time = 현재 tick - arrival time
			printf("[tick: %02d ] Dispatch to Process (ID: %d)\n", tick, info[CPU][0]);
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