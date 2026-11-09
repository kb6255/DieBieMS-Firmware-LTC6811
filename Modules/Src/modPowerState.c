#include "modPowerState.h"

modConfigGeneralConfigStructTypedef *modPowerStateGeneralConfigHandle;

bool modPowerStatePulsePowerDownDesired;
bool modPowerStateForceOnDesired;
bool modPowerStateButtonPressedVar;
bool modPowerStateLastButtonPressedVar;
bool modPowerStateLastButtonFirstPress;
bool modPowerStateButtonPulsToggleMode;
bool modPowerStatePowerModeDirectHCDelay;
uint32_t modPowerStateButtonPressedDuration;
uint32_t modPowerStateButtonPressedTimeStamp;
uint32_t modPowerStateStartupDelay;
uint32_t modPowerStatePowerDownTimeout;

void modPowerStateInit(PowerStateStateTypedef desiredPowerState) {
	uint32_t startupDelay = HAL_GetTick();
	modPowerStateStartupDelay = HAL_GetTick();
	modPowerStatePowerDownTimeout = HAL_GetTick();
	modPowerStatePulsePowerDownDesired = false;   // ���������Ĺػ�����1����ֵ��
	modPowerStateForceOnDesired  = false;  // ǿ�ƿ�������5����ֵ������״̬�ָ���
	modPowerStateButtonPressedVar = true;  // ��ť״̬��true=���£�
	modPowerStateButtonPulsToggleMode = true;   // �̰��л����ػ�ģʽ
	modPowerStateButtonPressedDuration = 0;    // ��ť�Ѱ��³���ʱ��
	modPowerStateButtonPressedTimeStamp = 0;    // ��ť����˲���ʱ���
	//1.所有变量归�?/�?默�?�值，�?保�??一次进�? modPowerStateTask() 时状态可预测�?
	driverHWPowerStateInit();   // ��ʼ��Ӳ��״̬��// 初�?�化�?件状�?
	 
	while(!modDelayTick1ms(&modPowerStateStartupDelay,20));	// Needed for power button signal to reach uC 
	modPowerStateLastButtonFirstPress = modPowerStateLastButtonPressedVar = driverHWPowerStateReadInput(P_STAT_BUTTON_INPUT);  // ��ʼ����ť״̬
	
	if(!modPowerStateLastButtonFirstPress) {// �����ť���ϵ�ʱ�����£��ȴ���ť�ͷ�?
		while(!modDelayTick1ms(&modPowerStateStartupDelay,100));//
	}
	modPowerStateSetState(desiredPowerState);// ���ó�ʼ��Դ״̬
};



void modPowerStateSetConfigHandle(modConfigGeneralConfigStructTypedef *generalConfigPointer) {
	modPowerStateGeneralConfigHandle = generalConfigPointer;
	
	modPowerStateButtonPulsToggleMode   = modPowerStateGeneralConfigHandle->pulseToggleButton;
	modPowerStatePowerModeDirectHCDelay = modPowerStateGeneralConfigHandle->togglePowerModeDirectHCDelay;
}

void modPowerStateTask(void) {
	bool tempButtonPressed = driverHWPowerStateReadInput(P_STAT_BUTTON_INPUT);// ��ȡ��ť״̬
	
	if(modPowerStateLastButtonPressedVar != tempButtonPressed) {// �����ť״�?�ı�
		if(modPowerStateLastButtonPressedVar){ 																	// If is was high and now low (actuated)
			modPowerStateLastButtonFirstPress = false;// ��ť�Ӱ���״̬�ͷ�
		}else{ 																																	// If is was low and now high (non actuated)
			modPowerStateButtonPressedTimeStamp = HAL_GetTick();// ��¼��ť����˲���ʱ���
		}
		modPowerStateLastButtonPressedVar = tempButtonPressed;// ���°�ť״̬
	}
	
	if(tempButtonPressed) {// �����ť������?
		modPowerStateButtonPressedDuration = HAL_GetTick() - modPowerStateButtonPressedTimeStamp;// ���㰴ť���µĳ���ʱ��
	
		if((modPowerStateButtonPressedDuration >= POWERBUTTON_POWERDOWN_THRESHOLD_TIME) && (modPowerStateLastButtonFirstPress == false) && modPowerStateGeneralConfigHandle->pulseToggleButton) {// �����ť����ʱ��?���ػ���ֵ�����ǵ�һ�ΰ��£��������̰��л����ػ�ģʽ
			modPowerStatePulsePowerDownDesired = true;
			modPowerStateButtonPressedDuration = 0;
		}
		
		if((modPowerStateButtonPressedDuration >= POWERBUTTON_FORCEON_THRESHOLD_TIME) && (modPowerStateLastButtonFirstPress == true) && modPowerStateGeneralConfigHandle->allowForceOn && modPowerStateGeneralConfigHandle->pulseToggleButton) {
			modPowerStateForceOnDesired = true;
			modPowerStateButtonPressedDuration = 0;
		}
		
		if((modPowerStateButtonPressedDuration > POWERBUTTON_DEBOUNCE_TIME) && (modPowerStateLastButtonFirstPress == false))
			modPowerStateButtonPressedVar = true;
		
		modPowerStatePowerDownTimeout = HAL_GetTick();
	}else{
		if(modDelayTick1ms(&modPowerStatePowerDownTimeout,500))
			modPowerStateButtonPressedVar = false;
	}
};

bool modPowerStateGetButtonPressedState(void) {
	return modPowerStateButtonPressedVar;
};

bool modPowerStateChargerDetected(void) {
	static bool chargeDetect = false;
	chargeDetect = driverHWPowerStateReadInput(P_STAT_CHARGE_DETECT);
	return chargeDetect;
};

bool modPowerStatePowerdownRequest(void) {
	bool returnValue = false;
	
	if(modPowerStateGeneralConfigHandle->pulseToggleButton){
		returnValue = modPowerStatePulsePowerDownDesired;
	}else{
		if(modPowerStateGeneralConfigHandle->togglePowerModeDirectHCDelay){
			returnValue = !modPowerStateButtonPressedVar;
		}else{
			returnValue = false;
		}
	}
	
	return returnValue;
};

bool modPowerStateForceOnRequest(void) {
	static bool firstTrigger = true;
	
	if(modPowerStateForceOnDesired && firstTrigger){
		modPowerStateForceOnDesired = false;
		firstTrigger = false;
		return true;
	}else{
		return false;
	}
};

void modPowerStateSetState(PowerStateStateTypedef newState) {
	driverHWPowerStateSetOutput(P_STAT_POWER_ENABLE,newState);
};

bool modPowerStateButtonPressedOnTurnon(void) {
	return modPowerStateLastButtonFirstPress;
};
