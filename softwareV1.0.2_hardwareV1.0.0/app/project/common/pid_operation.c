#include "PID_operation.h"

 PID handle_pid;
//为了防止积分项过度累积，引入积分项的限幅是一种常见的做法。
//限制积分项的幅值可以防止积分项过度增加，从而限制了系统的累积误差。这样可以避免系统过度响应或者不稳定。
volatile float abs_limit(volatile float value, volatile float ABS_MAX)   //积分限幅，设置最大值。
{
    if (value > ABS_MAX)
    {
        value = ABS_MAX;
    }
    if (value < -ABS_MAX)
    {
        value = -ABS_MAX;
    }

    return value;
}


//函数里传入指针，修改时会修改指针里的值。
float PID_Position_Calc(PID *pid, float Target_val, float Actual_val)  //位置式PID
{
    pid->Error = Target_val - Actual_val;      //与pid P系数相乘。比例误差值 当前差值=目标值-实际值
    
	#if 0
    if (pid->SumError >= pid->outputmax)
    {
        pid->SumError = pid->outputmax;
    }
	if (pid->SumError < 0)
	    pid->SumError = 0;
    else 
    {
        pid->SumError += pid->Error;    //与pid I系数相乘。稳态误差值 误差相加作为误差总和，给积分项
    }
    #endif
	pid->SumError += pid->Error;    //与pid I系数相乘。稳态误差值 误差相加作为误差总和，给积分项
	
	if (pid->SumError >=  pid->Integralmax)
    {
        pid->SumError =  pid->Integralmax;
    }
	
	if (pid->SumError <= 0)
    {
        pid->SumError = 0;
    }
	
    pid->DError = pid->Error - pid->LastError;   //与pid D系数相乘。 微分项-消除震荡
    pid->output =   pid->Kp * pid->Error +
                    abs_limit(pid->Ki * pid->SumError, pid->Integralmax) +
                    pid->Kd * pid->DError ;
    pid->LastError = pid->Error; //更新误差

    //限制输出最大值，防止出现突发意外。输出outputmax的最大值
    if (pid->output > pid->outputmax)
    {
        pid->output = pid->outputmax;
    }

//    if (pid->output < - pid->outputmax)
//    {
//        pid->output = -pid->outputmax;
//    }

	if (pid->output < - pid->outputmax)
    {
        pid->output = 0;
    }
	
    return pid->output;   //输出为pwm值
}

//PID初始化
void PID_Init(PID *pid, float Kp, float Ki, float Kd, float Limit_value)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->PrevError = pid->LastError = pid->Error = pid->SumError = pid->output =  0;
    pid->Integralmax = Limit_value;
    pid->outputmax  = Limit_value;
}

void PID_Clear(PID *pid)
{
    pid->PrevError = pid->LastError = pid->Error = pid->SumError = pid->output =  0;
}

void PID_Clear_I(PID *pid)
{
    pid->SumError = 0;
}


