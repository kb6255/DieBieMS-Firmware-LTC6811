#include "modPowerElectronics.h"

modPowerElectricsPackStateTypedef* modPowerElectronicsPackStateHandle;
modConfigGeneralConfigStructTypedef* modPowerElectronicsGeneralConfigHandle;
uint32_t modPowerElectronicsMeasureIntervalLastTick;

uint32_t modPowerElectronicsChargeRetryLastTick;
uint32_t modPowerElectronicsDisChargeLCRetryLastTick;
uint32_t modPowerElectronicsDisChargeHCRetryLastTick;
uint32_t modPowerElectronicsCellBalanceUpdateLastTick;
uint32_t modPowerElectronicsTempMeasureDelayLastTick;
uint32_t modPowerElectronicsChargeCurrentDetectionLastTick;
uint32_t modPowerElectronicsBalanceModeActiveLastTick;
uint8_t modPowerElectronicsUnderAndOverVoltageErrorCount;
driverLTC6811ConfigStructTypedef modPowerElectronicsLTCconfigStruct;
bool modPowerElectronicsAllowForcedOnState;
uint16_t modPowerElectronicsTemperatureArray[3];
uint16_t tempTemperature;
float modPowerElectronicsTempPackVoltage;
uint8_t modPowerElectronicsISLErrorCount;

// �������ӿ���ģ���ʼ��?:��������/״̬���?,���ô����������ʼ�����������״̬������Ӳ������
void modPowerElectronicsInit(modPowerElectricsPackStateTypedef* packState, modConfigGeneralConfigStructTypedef* generalConfigPointer)
{
    modPowerElectronicsGeneralConfigHandle = generalConfigPointer; // ����ͨ������ָ��,��ģ���ڲ�����
    modPowerElectronicsPackStateHandle = packState;                // ��������״ָ̬��,��ģ���ڲ�����
    modPowerElectronicsUnderAndOverVoltageErrorCount = 0;          // ������?/Ƿѹ�������?
    modPowerElectronicsAllowForcedOnState = false;                 // ��ֹǿ�ƿ���״̬
    modPowerElectronicsISLErrorCount = 0;                          // ���ISL�����������?

    // Init pack status
    modPowerElectronicsPackStateHandle->throttleDutyCharge = 0;                       // ��ʼ���������ռ�ձ�?
    modPowerElectronicsPackStateHandle->throttleDutyDischarge = 0;                    // ��ʼ���ŵ�����ռ�ձ�
    modPowerElectronicsPackStateHandle->SoC = 0.0f;                                   // ��ʼ��SoCΪ0
    modPowerElectronicsPackStateHandle->SoCCapacityAh = 0.0f;                         // ��ʼ��SoC����AhΪ0
    modPowerElectronicsPackStateHandle->operationalState = OP_STATE_INIT;             // ��ʼ������״̬Ϊ��ʼ״̬
    modPowerElectronicsPackStateHandle->packVoltage = 0.0f;                           // ��ʼ��������ѹΪ0
    modPowerElectronicsPackStateHandle->packCurrent = 0.0f;                           // ��ʼ����������Ϊ0
    modPowerElectronicsPackStateHandle->packPower = 0.0f;                             // ��ʼ������鹦���?0
    modPowerElectronicsPackStateHandle->loCurrentLoadCurrent = 0.0f;                  // ��ʼ���͵������ص���Ϊ0
    modPowerElectronicsPackStateHandle->loCurrentLoadVoltage = 0.0f;                  // ��ʼ���͵������ص�ѹΪ0
    modPowerElectronicsPackStateHandle->cellVoltageHigh = 0.0f;                       // ��ʼ����ص�ѹ���?0
    modPowerElectronicsPackStateHandle->cellVoltageLow = 0.0f;                        // ��ʼ����ص�ѹ���?0
    modPowerElectronicsPackStateHandle->cellVoltageAverage = 0.0;                     // ��ʼ����ص�ѹƽ���?0
    modPowerElectronicsPackStateHandle->disChargeDesired = false;                     // ��ʼ���ŵ�����Ϊfalse
    modPowerElectronicsPackStateHandle->disChargeLCAllowed = true;                    // ��ʼ���ŵ�LC����Ϊtrue
    modPowerElectronicsPackStateHandle->disChargeHCAllowed = true;                    // ��ʼ���ŵ�HC����Ϊtrue
    modPowerElectronicsPackStateHandle->preChargeDesired = false;                     // ��ʼ��Ԥ�������Ϊfalse
    modPowerElectronicsPackStateHandle->chargeDesired = false;                        // ��ʼ���������Ϊfalse
    modPowerElectronicsPackStateHandle->chargeAllowed = true;                         // ��ʼ���������Ϊtrue
    modPowerElectronicsPackStateHandle->safetyOverCANHCSafeNSafe = false;             // ��ʼ��CAN HC��ȫN��ȫΪfalse
    modPowerElectronicsPackStateHandle->chargeBalanceActive = false;                  // ��ʼ�������⼤��Ϊfalse
    modPowerElectronicsPackStateHandle->chargeCurrentDetected = false;                // ��ʼ�����������Ϊfalse
    modPowerElectronicsPackStateHandle->powerButtonActuated = false;                  // ��ʼ����Դ��ť����Ϊfalse
    modPowerElectronicsPackStateHandle->packInSOA = true;                             // ��ʼ�������SOA��Ϊtrue
    modPowerElectronicsPackStateHandle->watchDogTime = 255;                           // ��ʼ�����Ź�ʱ��Ϊ255
    modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_NORMAL; // ��ʼ����ز���״�?Ϊ����
    modPowerElectronicsPackStateHandle->temperatures[0] = 200.0f;                     // ��ʼ���¶�0Ϊ200
    modPowerElectronicsPackStateHandle->temperatures[1] = 200.0f;                     // ��ʼ���¶�1Ϊ200
    modPowerElectronicsPackStateHandle->temperatures[2] = 200.0f;                     // ��ʼ���¶�2Ϊ200
    modPowerElectronicsPackStateHandle->temperatures[3] = 200.0f;                     // ��ʼ���¶�3Ϊ200
    modPowerElectronicsPackStateHandle->tempBatteryHigh = 0.0f;                       // ��ʼ������¶ȸ��?0
    modPowerElectronicsPackStateHandle->tempBatteryLow = 0.0f;                        // ��ʼ������¶ȵ��?0
    modPowerElectronicsPackStateHandle->tempBatteryAverage = 0.0f;                    // ��ʼ������¶�ƽ���?0
    modPowerElectronicsPackStateHandle->tempBMSHigh = 0.0f;                           // ��ʼ��BMS�¶ȸ�Ϊ0
    modPowerElectronicsPackStateHandle->tempBMSLow = 0.0f;                            // ��ʼ��BMS�¶ȵ�Ϊ0
    modPowerElectronicsPackStateHandle->tempBMSAverage = 0.0f;                        // ��ʼ��BMS�¶�ƽ��Ϊ0

    modPowerElectronicsInitISL(); // ��ʼ��ISL28022

    // Init internal ADC
    driverHWADCInit();                                         // ��ʼ���ڲ�ADC
    driverHWSwitchesInit();                                    // ��ʼ��Ӳ������
    driverHWSwitchesSetSwitchState(SWITCH_DRIVER, SWITCH_SET); // ʹ��FET������

    // Init battery stack monitor
    driverLTC6811ConfigStructTypedef configStruct;                                                     // LTC6811���ýṹ��
    configStruct.ADCOPT = 0;                                                                           // ADCѡ��
    configStruct.REFON = 1;                                                                            // ʹ�ܲο���ѹ
    configStruct.GPIO[0] = false;                                                                      // GPIO0
    configStruct.GPIO[1] = false;                                                                      // GPIO1
    configStruct.GPIO[2] = true;                                                                       // GPIO2
    configStruct.GPIO[3] = false;                                                                      // GPIO3
    configStruct.GPIO[4] = false;                                                                      // GPIO4
    configStruct.CDCMode = 2;                                                                          // CDCģʽ
    configStruct.DisChargeEnableMask = 0x0000;                                                         // �ŵ�ʹ������
    configStruct.noOfCells = modPowerElectronicsGeneralConfigHandle->noOfCells;                        // �������?
    configStruct.CellUnderVoltageLimit = modPowerElectronicsGeneralConfigHandle->cellHardUnderVoltage; // ��ص�ѹ����?
    configStruct.CellOverVoltageLimit = modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage;   // ��ص�ѹ����?

    driverSWLTC6811Init(configStruct, TotalLTCICs); // ��ʼ��LTC6811

    modPowerElectronicsChargeCurrentDetectionLastTick = HAL_GetTick(); // ��ʼ�����������ʱ��?
    modPowerElectronicsBalanceModeActiveLastTick = HAL_GetTick();      // ��ʼ������ģʽ����ʱ��
};
// ������������
bool modPowerElectronicsTask(void)
{
    bool returnValue = false;

    if (modDelayTick1ms(&modPowerElectronicsMeasureIntervalLastTick, 100))
    { // ÿ100����ִ��һ�β���
        // ���� LTC �¶�����ת����ʱ�ļ�ʱ
        modPowerElectronicsTempMeasureDelayLastTick = HAL_GetTick();

        // �ɼ�С����ͨ·���ݲ�У����Ч�ԣ���������Ч��ִ�лָ�������
        driverSWISL28022GetBusVoltage(ISL28022_MASTER_ADDRES, ISL28022_MASTER_BUS, &modPowerElectronicsTempPackVoltage, 0.004f);
        if (fabs(modPowerElectronicsTempPackVoltage - modPowerElectronicsGeneralConfigHandle->noOfCells * modPowerElectronicsPackStateHandle->cellVoltageAverage) < 1.0f)
        { // If the error is smaller than one volt continue normal operation.
            modPowerElectronicsPackStateHandle->packVoltage = modPowerElectronicsTempPackVoltage;
            driverSWISL28022GetBusCurrent(ISL28022_MASTER_ADDRES, ISL28022_MASTER_BUS, &modPowerElectronicsPackStateHandle->loCurrentLoadCurrent, modPowerElectronicsGeneralConfigHandle->shuntLCOffset, modPowerElectronicsGeneralConfigHandle->shuntLCFactor);
            driverHWADCGetLoadVoltage(&modPowerElectronicsPackStateHandle->loCurrentLoadVoltage);
            modPowerElectronicsISLErrorCount = 0; // Reset error count.
        }
        else
        { // Error in voltage measurement.
            if (modPowerElectronicsISLErrorCount++ >= ISLErrorThreshold)
            { // Increase error count
                modPowerElectronicsISLErrorCount = ISLErrorThreshold;
                // Make BMS signal error state and power down.
            }
            else
            {
                modPowerElectronicsInitISL(); // Reinit I2C and ISL
            }
        }

        // Combine the two currents and calculate pack power.
        modPowerElectronicsPackStateHandle->packCurrent = modPowerElectronicsPackStateHandle->loCurrentLoadCurrent + modPowerElectronicsPackStateHandle->hiCurrentLoadCurrent;
        modPowerElectronicsPackStateHandle->packPower = modPowerElectronicsPackStateHandle->packCurrent * modPowerElectronicsPackStateHandle->packVoltage;

        // Check if LTC is still running
        driverSWLTC6811WakeUp(TotalLTCICs);                             // ÿ�β���ǰ����LTC6811,��ֹоƬ�������ߵ����������
        driverSWLTC6811ReadConfig(&modPowerElectronicsLTCconfigStruct); // ��ȡLTC6811���üĴ���ֵ
        if (!modPowerElectronicsLTCconfigStruct.CDCMode)                // ���LTC6811�Ƿ���������
            driverSWLTC6811ReInit();                                    // ���³�ʼ��LTC6811
        else
            driverSWLTC6811ReadCellVoltages(modPowerElectronicsPackStateHandle->cellVoltagesIndividual); // ��ȡLTC6811��ص��?

        // ���? LTC �ڷǳ��״�?���Ƿ����÷ŵ����?
        if (!modPowerElectronicsPackStateHandle->chargeDesired && modPowerElectronicsLTCconfigStruct.DisChargeEnableMask)
            driverSWLTC6811ReInit(); // Something went wrong, reinit the battery stack monitor.

        // �ɼ� LTC �¶�����
        driverSWLTC6811ReadTempVoltages(modPowerElectronicsTemperatureArray);
        modPowerElectronicsPackStateHandle->temperatures[0] = driverSWLTC6811ConvertTemperatureExt(modPowerElectronicsTemperatureArray[0], modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupLTCExt], modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupLTCExt], modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupLTCExt], 25.0f);
        modPowerElectronicsPackStateHandle->temperatures[1] = driverSWLTC6811ConvertTemperatureExt(modPowerElectronicsTemperatureArray[1], modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupLTCExt], modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupLTCExt], modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupLTCExt], 25.0f);
        modPowerElectronicsPackStateHandle->temperatures[2] = driverSWLTC6811ConvertTemperatureInt(modPowerElectronicsTemperatureArray[2]);
        // get STM32 ADC NTC temp
        driverHWADCGetNTCValue(&modPowerElectronicsPackStateHandle->temperatures[3], modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupMasterPCB], modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupMasterPCB], modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupMasterPCB], 25.0f);

        // Calculate temperature statisticks
        modPowerElectronicsCalcTempStats();

        // When temperature and cellvoltages are known calculate charge and discharge throttle.
        modPowerElectronicsCalcThrottle();

        // Do the balancing task
        modPowerElectronicsSubTaskBalaning();

        // Measure cell voltages
        driverSWLTC6811StartCellVoltageConversion();
        driverSWLTC6811ResetCellVoltageRegisters();

        // Check and respond to the measured voltage values
        modPowerElectronicsSubTaskVoltageWatch();

        // Check and respond to the measured temperature values
        // modPowerElectronicsSubTaskTemperatureWatch(); // ��鲢��Ӧ�������¶�ֵ

        // Check and determine whether or not there is a charge current and we need to balance.
        if (modPowerElectronicsPackStateHandle->packCurrent >= modPowerElectronicsGeneralConfigHandle->chargerEnabledThreshold)
        {
            if (modDelayTick1ms(&modPowerElectronicsChargeCurrentDetectionLastTick, 5000))
            {
                modPowerElectronicsPackStateHandle->chargeBalanceActive = modPowerElectronicsGeneralConfigHandle->allowChargingDuringDischarge;
                modPowerElectronicsPackStateHandle->chargeCurrentDetected = true; // Charge current is detected after 2 seconds
            }

            if (modPowerElectronicsPackStateHandle->chargeCurrentDetected)
            {
                modPowerElectronicsResetBalanceModeActiveTimeout();
            }
        }
        else
        {
            modPowerElectronicsPackStateHandle->chargeCurrentDetected = false;
            modPowerElectronicsChargeCurrentDetectionLastTick = HAL_GetTick();
        }
        // TODO: have balance time configureable
        if (modDelayTick1ms(&modPowerElectronicsBalanceModeActiveLastTick, 10 * 60 * 1000))
        { // When a charge current is derected, balance for 10 minutes
            modPowerElectronicsPackStateHandle->chargeBalanceActive = false;
        }

        modPowerElectronicsPackStateHandle->powerButtonActuated = modPowerStateGetButtonPressedState();

        returnValue = true;
    }
    else
        returnValue = false;

    if (modDelayTick1msNoRST(&modPowerElectronicsTempMeasureDelayLastTick, 50)) // ÿ50��������һ���¶ȵ�ѹת��
        driverSWLTC6811StartTemperatureVoltageConversion();                     // �����¶ȵ�ѹת��

    return returnValue;
};

void modPowerElectronicsAllowForcedOn(bool allowedState)
{
    modPowerElectronicsAllowForcedOnState = allowedState;
}

void modPowerElectronicsSetPreCharge(bool newState)
{
    static bool preChargeLastState = false;

    if (preChargeLastState != newState)
    {
        preChargeLastState = newState;

        if (newState)
            driverHWSwitchesSetSwitchState(SWITCH_DRIVER, SWITCH_SET);

        modPowerElectronicsPackStateHandle->preChargeDesired = newState;
        modPowerElectronicsUpdateSwitches();
    }
};

bool modPowerElectronicsSetDisCharge(bool newState)
{
    static bool dischargeLastState = false;

    if (dischargeLastState != newState)
    {
        if (newState)
            driverHWSwitchesSetSwitchState(SWITCH_DRIVER, SWITCH_SET);

        modPowerElectronicsPackStateHandle->disChargeDesired = newState;
        modPowerElectronicsUpdateSwitches();
        dischargeLastState = newState;
    }

    if (modPowerElectronicsPackStateHandle->loCurrentLoadVoltage < PRECHARGE_PERCENTAGE * (modPowerElectronicsPackStateHandle->packVoltage)) // Prevent turn on with to low output voltage
        return false;                                                                                                                        // Load voltage to low (output not precharged enough)
    else
        return true;
};

void modPowerElectronicsSetCharge(bool newState)
{
    static bool chargeLastState = false;

    if (chargeLastState != newState)
    {
        chargeLastState = newState;

        if (newState)
            driverHWSwitchesSetSwitchState(SWITCH_DRIVER, SWITCH_SET);

        modPowerElectronicsPackStateHandle->chargeDesired = newState;
        modPowerElectronicsUpdateSwitches();
    }
};

void modPowerElectronicsDisableAll(void)
{
    if (modPowerElectronicsPackStateHandle->disChargeDesired | modPowerElectronicsPackStateHandle->preChargeDesired | modPowerElectronicsPackStateHandle->chargeDesired)
    {
        modPowerElectronicsPackStateHandle->disChargeDesired = false;
        modPowerElectronicsPackStateHandle->preChargeDesired = false;
        modPowerElectronicsPackStateHandle->chargeDesired = false;
        driverHWSwitchesDisableAll();
    }
};

void modPowerElectronicsCalculateCellStats(void)
{
    float cellVoltagesSummed = 0.0f;
    modPowerElectronicsPackStateHandle->cellVoltageHigh = 0.0f;
    modPowerElectronicsPackStateHandle->cellVoltageLow = 10.0f;

    for (uint8_t cellPointer = 0; cellPointer < modPowerElectronicsGeneralConfigHandle->noOfCells; cellPointer++)
    {
        cellVoltagesSummed += modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;

        if (modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage > modPowerElectronicsPackStateHandle->cellVoltageHigh)
            modPowerElectronicsPackStateHandle->cellVoltageHigh = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;

        if (modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage < modPowerElectronicsPackStateHandle->cellVoltageLow)
            modPowerElectronicsPackStateHandle->cellVoltageLow = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;
    }

    modPowerElectronicsPackStateHandle->cellVoltageAverage = cellVoltagesSummed / modPowerElectronicsGeneralConfigHandle->noOfCells;
    modPowerElectronicsPackStateHandle->cellVoltageMisMatch = modPowerElectronicsPackStateHandle->cellVoltageHigh - modPowerElectronicsPackStateHandle->cellVoltageLow;
};

void modPowerElectronicsSubTaskBalaning(void)
{
    static uint32_t delayTimeHolder = 100;
    static uint16_t lastCellBalanceRegister = 0;
    static bool delaytoggle = false;
    uint16_t cellBalanceMaskEnableRegister = 0;
    driverLTC6811CellsTypedef sortedCellArray[modPowerElectronicsGeneralConfigHandle->noOfCells];

    if (modDelayTick1ms(&modPowerElectronicsCellBalanceUpdateLastTick, delayTimeHolder))
    {
        delaytoggle ^= true;
        delayTimeHolder = delaytoggle ? modPowerElectronicsGeneralConfigHandle->cellBalanceUpdateInterval : 200;

        if (delaytoggle)
        {
            for (int k = 0; k < modPowerElectronicsGeneralConfigHandle->noOfCells; k++)
            {
                sortedCellArray[k] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[k]; // This will contain the voltages that are unloaded by balance resistors
            }

            modPowerElectronicsSortCells(sortedCellArray, modPowerElectronicsGeneralConfigHandle->noOfCells);

            if ((modPowerElectronicsPackStateHandle->chargeDesired && !modPowerElectronicsPackStateHandle->disChargeDesired) || modPowerElectronicsPackStateHandle->chargeBalanceActive || !modPowerElectronicsPackStateHandle->chargeAllowed)
            { // Check if charging is desired
                for (uint8_t i = 0; i < modPowerElectronicsGeneralConfigHandle->maxSimultaneousDischargingCells; i++)
                {
                    if (sortedCellArray[i].cellVoltage >= (modPowerElectronicsPackStateHandle->cellVoltageLow + modPowerElectronicsGeneralConfigHandle->cellBalanceDifferenceThreshold))
                    {
                        if (sortedCellArray[i].cellVoltage >= modPowerElectronicsGeneralConfigHandle->cellBalanceStart)
                        {
                            cellBalanceMaskEnableRegister |= (1 << sortedCellArray[i].cellNumber);
                        }
                    };
                }
            }
        }

        modPowerElectronicsPackStateHandle->cellBalanceResistorEnableMask = cellBalanceMaskEnableRegister;

        if (lastCellBalanceRegister != cellBalanceMaskEnableRegister)
            driverSWLTC6811EnableBalanceResistors(cellBalanceMaskEnableRegister); // ʹ�ܵ�ؾ������
        lastCellBalanceRegister = cellBalanceMaskEnableRegister;
    }
};

void modPowerElectronicsSubTaskVoltageWatch(void)
{
    static bool lastdisChargeLCAllowed = false;
    static bool lastChargeAllowed = false;
    uint16_t hardUnderVoltageFlags, hardOverVoltageFlags;

    driverSWLTC6811ReadVoltageFlags(&hardUnderVoltageFlags, &hardOverVoltageFlags); // ��ȡ��ص�ѹ���?
    modPowerElectronicsCalculateCellStats();

    if (modPowerElectronicsPackStateHandle->packOperationalCellState != PACK_STATE_ERROR_HARD_CELLVOLTAGE)
    {
        // Handle soft cell voltage limits
        // Low current
        if (modPowerElectronicsPackStateHandle->cellVoltageLow <= modPowerElectronicsGeneralConfigHandle->cellLCSoftUnderVoltage)
        {
            modPowerElectronicsPackStateHandle->disChargeLCAllowed = false;
            modPowerElectronicsDisChargeLCRetryLastTick = HAL_GetTick();
        }

        // High current
        if (modPowerElectronicsPackStateHandle->cellVoltageLow <= modPowerElectronicsGeneralConfigHandle->cellHCSoftUnderVoltage)
        {
            modPowerElectronicsPackStateHandle->disChargeHCAllowed = false;
            modPowerElectronicsDisChargeHCRetryLastTick = HAL_GetTick();
        }

        if (modPowerElectronicsPackStateHandle->cellVoltageHigh >= modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage)
        {
            modPowerElectronicsPackStateHandle->chargeAllowed = false;
            modPowerElectronicsChargeRetryLastTick = HAL_GetTick();
        }

        // Low current
        if (modPowerElectronicsPackStateHandle->cellVoltageLow >= (modPowerElectronicsGeneralConfigHandle->cellLCSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->hysteresisDischarge))
        {
            if (modDelayTick1ms(&modPowerElectronicsDisChargeLCRetryLastTick, modPowerElectronicsGeneralConfigHandle->timeoutDischargeRetry))
                modPowerElectronicsPackStateHandle->disChargeLCAllowed = true;
        }

        // High current
        if (modPowerElectronicsPackStateHandle->cellVoltageLow >= (modPowerElectronicsGeneralConfigHandle->cellHCSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->hysteresisDischarge))
        {
            if (modDelayTick1ms(&modPowerElectronicsDisChargeHCRetryLastTick, modPowerElectronicsGeneralConfigHandle->timeoutDischargeRetry))
                modPowerElectronicsPackStateHandle->disChargeHCAllowed = true;
        }

        if (modPowerElectronicsPackStateHandle->cellVoltageHigh <= (modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->hysteresisCharge))
        {
            if (modDelayTick1ms(&modPowerElectronicsChargeRetryLastTick, modPowerElectronicsGeneralConfigHandle->timeoutChargeRetry))
                modPowerElectronicsPackStateHandle->chargeAllowed = true;
        }

        if (modPowerElectronicsPackStateHandle->chargeAllowed && modPowerElectronicsPackStateHandle->disChargeLCAllowed)
            modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_NORMAL;
        else
            modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_ERROR_SOFT_CELLVOLTAGE;
    }

    // Handle hard cell voltage limits
    if (hardUnderVoltageFlags || hardOverVoltageFlags || (modPowerElectronicsPackStateHandle->packVoltage > modPowerElectronicsGeneralConfigHandle->noOfCells * modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage))
    {
        if (modPowerElectronicsUnderAndOverVoltageErrorCount++ > modPowerElectronicsGeneralConfigHandle->maxUnderAndOverVoltageErrorCount)
            modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_ERROR_HARD_CELLVOLTAGE;
        modPowerElectronicsPackStateHandle->disChargeLCAllowed = false;
        modPowerElectronicsPackStateHandle->chargeAllowed = false;
    }
    else
        modPowerElectronicsUnderAndOverVoltageErrorCount = 0;

    // update outputs directly if needed
    if ((lastChargeAllowed != modPowerElectronicsPackStateHandle->chargeAllowed) || (lastdisChargeLCAllowed != modPowerElectronicsPackStateHandle->disChargeLCAllowed))
    {
        lastChargeAllowed = modPowerElectronicsPackStateHandle->chargeAllowed;
        lastdisChargeLCAllowed = modPowerElectronicsPackStateHandle->disChargeLCAllowed;
        modPowerElectronicsUpdateSwitches();
    }
};

// Update switch states, should be called after every desired/allowed switch state change
void modPowerElectronicsUpdateSwitches(void)
{
    // Do the actual power switching in here

    // Handle pre charge output
    if (modPowerElectronicsPackStateHandle->preChargeDesired && (modPowerElectronicsPackStateHandle->disChargeLCAllowed || modPowerElectronicsAllowForcedOnState))
        driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE, (driverHWSwitchesStateTypedef)SWITCH_SET);
    else
        driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE, (driverHWSwitchesStateTypedef)SWITCH_RESET);

    // Handle discharge output
    if (modPowerElectronicsPackStateHandle->disChargeDesired && (modPowerElectronicsPackStateHandle->disChargeLCAllowed || modPowerElectronicsAllowForcedOnState))
        driverHWSwitchesSetSwitchState(SWITCH_DISCHARGE, (driverHWSwitchesStateTypedef)SWITCH_SET);
    else
        driverHWSwitchesSetSwitchState(SWITCH_DISCHARGE, (driverHWSwitchesStateTypedef)SWITCH_RESET);

    // Handle charge input
    if (modPowerElectronicsPackStateHandle->chargeDesired && modPowerElectronicsPackStateHandle->chargeAllowed)
        driverHWSwitchesSetSwitchState(SWITCH_CHARGE, (driverHWSwitchesStateTypedef)SWITCH_SET);
    else
        driverHWSwitchesSetSwitchState(SWITCH_CHARGE, (driverHWSwitchesStateTypedef)SWITCH_RESET);
};

void modPowerElectronicsSortCells(driverLTC6811CellsTypedef* cells, uint8_t cellCount)
{
    int i, j;
    driverLTC6811CellsTypedef value;

    for (i = 0; i < (cellCount - 1); i++)
    {
        for (j = 0; j < (cellCount - i - 1); j++)
        {
            if (cells[j].cellVoltage < cells[j + 1].cellVoltage)
            {
                value = cells[j + 1];
                cells[j + 1] = cells[j];
                cells[j] = value;
            }
        }
    }
};

void modPowerElectronicsCalcTempStats(void)
{
    uint8_t sensorPointer;

    // Battery
    float tempBatteryMax;
    float tempBatteryMin;
    float tempBatterySum = 0.0f;
    uint8_t tempBatterySumCount = 0;

    // BMS
    float tempBMSMax;
    float tempBMSMin;
    float tempBMSSum = 0.0f;
    uint8_t tempBMSSumCount = 0;

    if (modPowerElectronicsGeneralConfigHandle->tempEnableMaskBattery)
    {
        tempBatteryMax = -100.0f;
        tempBatteryMin = 100.0f;
    }
    else
    {
        tempBatteryMax = 0.0f;
        tempBatteryMin = 0.0f;
    }

    if (modPowerElectronicsGeneralConfigHandle->tempEnableMaskBMS)
    {
        tempBMSMax = -100.0f;
        tempBMSMin = 100.0f;
    }
    else
    {
        tempBMSMax = 0.0f;
        tempBMSMin = 0.0f;
    }

    for (sensorPointer = 0; sensorPointer < 16; sensorPointer++)
    {
        // Battery temperatures
        if (modPowerElectronicsGeneralConfigHandle->tempEnableMaskBattery & (1 << sensorPointer))
        {
            if (modPowerElectronicsPackStateHandle->temperatures[sensorPointer] > tempBatteryMax)
                tempBatteryMax = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];

            if (modPowerElectronicsPackStateHandle->temperatures[sensorPointer] < tempBatteryMin)
                tempBatteryMin = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];

            tempBatterySum += modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
            tempBatterySumCount++;
        }

        // BMS temperatures
        if (modPowerElectronicsGeneralConfigHandle->tempEnableMaskBMS & (1 << sensorPointer))
        {
            if (modPowerElectronicsPackStateHandle->temperatures[sensorPointer] > tempBMSMax)
                tempBMSMax = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];

            if (modPowerElectronicsPackStateHandle->temperatures[sensorPointer] < tempBMSMin)
                tempBMSMin = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];

            tempBMSSum += modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
            tempBMSSumCount++;
        }
    }

    // Battery temperatures
    modPowerElectronicsPackStateHandle->tempBatteryHigh = tempBatteryMax;
    modPowerElectronicsPackStateHandle->tempBatteryLow = tempBatteryMin;
    if (tempBatterySumCount)
        modPowerElectronicsPackStateHandle->tempBatteryAverage = tempBatterySum / tempBatterySumCount;
    else
        modPowerElectronicsPackStateHandle->tempBatteryAverage = 0.0f;

    // BMS temperatures
    modPowerElectronicsPackStateHandle->tempBMSHigh = tempBMSMax;
    modPowerElectronicsPackStateHandle->tempBMSLow = tempBMSMin;
    if (tempBMSSumCount)
        modPowerElectronicsPackStateHandle->tempBMSAverage = tempBMSSum / tempBMSSumCount;
    else
        modPowerElectronicsPackStateHandle->tempBMSAverage = 0.0f;
};

void modPowerElectronicsCalcThrottle(void)
{
    uint8_t calculatedChargeThrottle = 0;
    uint8_t calculatedDisChargeThrottle = 0;

    static uint8_t filteredChargeThrottle = 0;
    static uint8_t filteredDisChargeThrottle = 0;

    float inputLowerLimitCharge = modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->cellThrottleUpperMargin - modPowerElectronicsGeneralConfigHandle->cellThrottleUpperStart;
    float inputUpperLimitCharge = modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->cellThrottleUpperMargin;
    float outputLowerLimitCharge = 100.0f;
    float outputUpperLimitCharge = 10.0f;

    float inputLowerLimitDisCharge = modPowerElectronicsGeneralConfigHandle->cellLCSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->cellThrottleLowerMargin;
    float inputUpperLimitDisCharge = modPowerElectronicsGeneralConfigHandle->cellLCSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->cellThrottleLowerMargin + modPowerElectronicsGeneralConfigHandle->cellThrottleLowerStart;
    float outputLowerLimitDisCharge = 5.0f;
    float outputUpperLimitDisCharge = 100.0f;

    // Calculate (dis)charge throttle
    calculatedChargeThrottle = (uint8_t)modPowerElectronicsMapVariableFloat(modPowerElectronicsPackStateHandle->cellVoltageHigh, inputLowerLimitCharge, inputUpperLimitCharge, outputLowerLimitCharge, outputUpperLimitCharge);
    calculatedDisChargeThrottle = (uint8_t)modPowerElectronicsMapVariableFloat(modPowerElectronicsPackStateHandle->cellVoltageLow, inputLowerLimitDisCharge, inputUpperLimitDisCharge, outputLowerLimitDisCharge, outputUpperLimitDisCharge);

    // Filter the calculated throttle
    if (calculatedChargeThrottle > filteredChargeThrottle)
    {
        if ((calculatedChargeThrottle - filteredChargeThrottle) > modPowerElectronicsGeneralConfigHandle->throttleChargeIncreaseRate)
            filteredChargeThrottle += modPowerElectronicsGeneralConfigHandle->throttleChargeIncreaseRate;
        else
            filteredChargeThrottle = calculatedChargeThrottle;
    }
    else
    {
        filteredChargeThrottle = calculatedChargeThrottle;
    }

    if (calculatedDisChargeThrottle > filteredDisChargeThrottle)
    {
        if ((calculatedDisChargeThrottle - filteredDisChargeThrottle) > modPowerElectronicsGeneralConfigHandle->throttleDisChargeIncreaseRate)
            filteredDisChargeThrottle += modPowerElectronicsGeneralConfigHandle->throttleDisChargeIncreaseRate;
        else
            filteredDisChargeThrottle = calculatedDisChargeThrottle;
    }
    else
    {
        filteredDisChargeThrottle = calculatedDisChargeThrottle;
    }

    // Output the filtered output
    if (modPowerElectronicsPackStateHandle->chargeAllowed)
        modPowerElectronicsPackStateHandle->throttleDutyCharge = filteredChargeThrottle;
    else
        modPowerElectronicsPackStateHandle->throttleDutyCharge = 0;

    if (modPowerElectronicsPackStateHandle->disChargeLCAllowed)
        modPowerElectronicsPackStateHandle->throttleDutyDischarge = filteredDisChargeThrottle;
    else
        modPowerElectronicsPackStateHandle->throttleDutyDischarge = 0;
}

int32_t modPowerElectronicsMapVariableInt(int32_t inputVariable, int32_t inputLowerLimit, int32_t inputUpperLimit, int32_t outputLowerLimit, int32_t outputUpperLimit)
{
    inputVariable = inputVariable < inputLowerLimit ? inputLowerLimit : inputVariable;
    inputVariable = inputVariable > inputUpperLimit ? inputUpperLimit : inputVariable;

    return (inputVariable - inputLowerLimit) * (outputUpperLimit - outputLowerLimit) / (inputUpperLimit - inputLowerLimit) + outputLowerLimit;
}

float modPowerElectronicsMapVariableFloat(float inputVariable, float inputLowerLimit, float inputUpperLimit, float outputLowerLimit, float outputUpperLimit)
{
    inputVariable = inputVariable < inputLowerLimit ? inputLowerLimit : inputVariable;
    inputVariable = inputVariable > inputUpperLimit ? inputUpperLimit : inputVariable;

    return (inputVariable - inputLowerLimit) * (outputUpperLimit - outputLowerLimit) / (inputUpperLimit - inputLowerLimit) + outputLowerLimit;
}

void modPowerElectronicsInitISL(void)
{
    // Init BUS monitor
    driverSWISL28022InitStruct ISLInitStruct;         // ISL28022��ʼ���ṹ��
    ISLInitStruct.ADCSetting = ADC_128_64010US;       // ADC����
    ISLInitStruct.busVoltageRange = BRNG_60V_1;       // ��ѹ��ѹ��Χ
    ISLInitStruct.currentShuntGain = PGA_4_160MV;     // ������������
    ISLInitStruct.Mode = MODE_SHUNTANDBUS_CONTINIOUS; // ģʽ
    driverSWISL28022Init(ISL28022_MASTER_ADDRES, ISL28022_MASTER_BUS, ISLInitStruct);
}

void modPowerElectronicsCheckPackSOA(void)
{
    bool packOutsideLimits = false;

    if (modPowerElectronicsGeneralConfigHandle->tempEnableMaskBMS)
    {
        packOutsideLimits |= (modPowerElectronicsPackStateHandle->tempBMSHigh > 70.0f) ? true : false;
    }

    if (modPowerElectronicsGeneralConfigHandle->tempEnableMaskBattery)
    {
        packOutsideLimits |= (modPowerElectronicsPackStateHandle->tempBatteryHigh > 70.0f) ? true : false;
    }

    packOutsideLimits |= (modPowerElectronicsISLErrorCount >= ISLErrorThreshold) ? true : false;

    // TODO: timout when restoring SOA state.
    modPowerElectronicsPackStateHandle->packInSOA = packOutsideLimits;
}

bool modPowerElectronicsHCSafetyCANAndPowerButtonCheck(void)
{
    if (modPowerElectronicsGeneralConfigHandle->useCANSafetyInput)
        return (modPowerElectronicsPackStateHandle->safetyOverCANHCSafeNSafe && modPowerElectronicsPackStateHandle->powerButtonActuated);
    else
        return true;
}

void modPowerElectronicsResetBalanceModeActiveTimeout(void)
{
    modPowerElectronicsBalanceModeActiveLastTick = HAL_GetTick();
}
