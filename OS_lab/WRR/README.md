# WRR algorithm based on task QoS
A weighted RR scheduling algorithm based on priority and real-time task QoS in small devices.

## Abstract

The selected essay [1] proposes a weighted Round Robin (WRR) scheduling algorithm based on Quality of Service (QoS) to handle real-time tasks on lightweight devices. In this paper, we reproduce the WRR algorithm in a popular operating system and analyse its performance. The algorithm utilizes the Token Bucket model (TSPEC) to anticipate the burst time of tasks and calculates different quantum times depending on the priority and QoS requirements of tasks. The algorithm is compared with the traditional Round Robin (RR) scheduling algorithm by simulation and evaluation. The evaluation results show that the algorithm can dramatically reduce the waiting and turnaround time of high-priority tasks, and improve the CPU response rate and resource utilization. Additionally, the paper proposes some improvements to the algorithm to make it adaptable to different task arrival times and total times, which is closer to the real situation.


## Chapter 1 Introduction

Unlike common devices, most tasks that lightweight devices need to execute are real-time tasks [2]. This means that lightweight devices have a highly variable task generating rate (sometimes, a burst of tasks are generated in a short time, and sometimes there are no tasks for a long time). As a result, the task queue for RR scheduling can become either too long or too short, leading to a decrease in CPU response rate. To address this issue, it is necessary to regulate the rate of task entry within a certain range. The author introduces a rate control model based on the Token Bucket model [ 3 ].

## Other chapters
Please see [PDF version](https://github.com/Aki-Hoyue/projects/blob/main/OS_lab/WRR/Report_of_WRR_algorithm_based_on_task_QoS.pdf). <br /><br />

**Reference**
[1]: Kim, S. (2017b). QoS provisioning of a task-scheduling algorithm for lightweight devices. *Journal of Parallel and Distributed Computing, 107, 67–75*. [https://doi.org/10.1016/j.jpdc.2017.04.010](https://doi.org/10.1016/j.jpdc.2017.04.010) <br />
[2]: Leung, J. Y., & Whitehead, J. (1982). On the complexity of fixed-priority scheduling of periodic, real-time tasks. *Performance Evaluation, 2(4), 237–250*. [https://doi.org/10.1016/0166-5316(82)90024-4](https://doi.org/10.1016/0166-5316(82)90024-4) <br />
[3]: Huawei Support. (2021). Traffic metering and token bucket mechanism. [https://support.huawei.com/enterprise/en/doc/EDOC1000178175/f6e567c8/traffic-metering-and-token-bucket-mechanism](https://support.huawei.com/enterprise/en/doc/EDOC1000178175/f6e567c8/traffic-metering-and-token-bucket-mechanism) <br />
[4]: Huawei Support. (2022). What Is Quality of Service (QoS)? [https://support.huawei.com/enterprise/en/doc/EDOC1100086518](https://support.huawei.com/enterprise/en/doc/EDOC1100086518)
[5]: Levis, P., Madden, S., Polastre, J., Szewczyk, R., Whitehouse, K., Woo, A., Hill, J., Welsh, M., Brewer, E., & Culler, D. (2005). TinyOS: an operating system for sensor networks. In *Springer eBooks* (pp. 115–148). [https://doi.org/10.1007/3-540-27139-2_7](https://doi.org/10.1007/3-540-27139-2_7) <br />
[6]: Aki-Hoyue, 0216Feng & LiangPeng03. (2023). Reproduction of WRR algorithm and improvement. [https://github.com/Aki-Hoyue/projects/tree/main/OS_lab/WRR](https://github.com/Aki-Hoyue/projects/tree/main/OS_lab/WRR) <br />


