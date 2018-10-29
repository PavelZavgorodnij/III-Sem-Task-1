#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<string>
#include<vector>
#include<time.h>
#include<malloc.h>
#include<fcntl.h>
#include<string.h>
#include<sys/wait.h>
using namespace std;
//------------------------------------------------------------------------
string get_string(FILE *fp){
	string ret;
	for(int c = getc(fp); c != '\n' && c != EOF; c = getc(fp))
		ret.push_back(c);
	return ret;
}
//------------------------------------------------------------------------
int for_sleep(int h, int m, int s, int h0, int m0, int s0){
	if (h == 1 && m >= 0){
		int ret = 60 * 24 + 60 * (m - m0) + (s - s0);
		ret = ret % (60 * 24);
		return ret;
	}
	if (h == -1 && m == -1){
		int ret = 60 * 25 + (s - s0);
		ret = ret % (60 * 24);
		ret = ret % 60;
		return ret;
	}
	if (h >= 0 && m == -1){
		if (h < h0) return -1;
		if (h == h0){
			int ret = 60 + (s - s0);
			ret = ret % 60;
			return ret;
		}
		m = 0;
	}
	int ret = 60 * 24 * (h - h0) + 60 * (m - m0) + (s - s0);
	return (ret >= 0) ? ret : -1;
}
//-------------------------------------------------------------------------
int main(){
	pid_t top_manager = fork();
	bool full_exit = false;
	if(top_manager == 0) for(;;)){
		if(full_exit) break;
		struct stat mycron_stat;
		stat("mycrontab", &mycron_stat);
		FILE *mycron = fopen("mycrontab", "r");
		vector<pid_t>forks;
		for(;;){
			vector<char *>args;
			vector<int>times;
			string str = get_string(mycron);
			if(str.size() == 0) break;
			if(str == "Hvatit!!!"){
				full_exit = true;
				break;
			}
			bool time_block = true;
			int shedule[3];
			for (int j = 0; j < 3; j++) shedule[j] = 0;
			int shed_step = 0;
			string word;
			for (int i = 0; i < str.size(); i++){
				if(time_block){
					if(str[i] == ' ') time_block = false;
					else if(str[i] == ':') shed_step++;
					else if(str[i] == '*') shedule[shed_step] = -1;
					else{
						shedule[shed_step] *= 10;
						shedule[shed_step] += (str[i] - '0');
					}
				} else {
					if(str[i] == ' '){
						args.push_back(strdup(word.c_str()));
						word.resize(0);
					} else word.push_back(str[i]);
				}
			}
			args.push_back(strdup(word.c_str()));
			args.push_back(NULL);
			forks.push_back(fork());
			if(forks[forks.size()-1] == 0){
				fclose(mycron);
				bool ifexit = false;
				for(;;){
					if(ifexit) break;
					time_t my_time = time(NULL);
					struct tm* my_tm = localtime(&my_time);
					int sleep_time = for_sleep(shedule[0], shedule[1], shedule[2], my_tm->tm_hour, my_tm->tm_min, my_tm->tm_sec);
					if (sleep_time > 0){
						sleep((unsigned int)sleep_time);
						pid_t pidinfor = fork();
						struct stat change;
						stat("mycrontab", &change);
						if(mycron_stat.st_mtime == change.st_mtime){
							if(pidinfor == 0){
								execvp(args[0], &args[0]);
								perror(args[0]);	
								for (size_t i = 0; i < args.size(); i++) free(args[i]);
								return 0;
							}							
						} else ifexit = true;
						int statusss;
						waitpid(pidinfor, &statusss, 0);
						sleep(5);	
					}
					else{
						printf("LoL, you opozdaLLL!!!\n\n");
						break;
					} 
				}
				for (size_t i = 0; i < args.size(); i++) free(args[i]);
				return 0;
			} else for (size_t i = 0; i < args.size(); i++) free(args[i]);
		}
		for (int i = 0; i < forks.size(); i++){
			int status;
			waitpid(forks[i], &status, 0);
		}
		fclose(mycron);
	}
}








































