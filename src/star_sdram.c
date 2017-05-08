#include "stm32f469xx.h"
#include "star_sdram.h"

//#define ENABLE_SDRAM_DMA
//#define SDRAM_32BIT_ACCESS

#if defined(SDRAM_32BIT_ACCESS) && !defined(BOARD_DISCO469)
    #error "SDRAM 32bit data bus is available only on SMT32F469-DISCO board"
#endif

#define REFRESH_COUNT                            ((uint32_t)0x0569)
#define  SDRAM_TIMEOUT                           ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)
#ifdef ENABLE_SDRAM_DMA
    #define __DMAx_CLK_ENABLE               __HAL_RCC_DMA2_CLK_ENABLE
    #define SDRAM_DMAx_CHANNEL              DMA_CHANNEL_0
    #define SDRAM_DMAx_STREAM               HAL_DMA2_Stream0
    #define SDRAM_DMAx_IRQn                 DMA2_Stream0_IRQn
    #define SDRAM_DMAx_IRQHandler           DMA2_Stream0_IRQHandler
#endif

static SDRAM_HandleTypeDef sdramHandle;

static void InitSeq(uint32_t RefreshCount)
{
    FMC_SDRAM_CommandTypeDef Command;

    __IO uint32_t tmpmrd = 0;

    /* Step 1: Configure a clock configuration enable command */
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

    /* Step 2: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 3: Configure a PALL (precharge all) command */
    Command.CommandMode            = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

    /* Step 4: Configure an Auto Refresh command */
    Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 8;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

    /* Step 5: Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |\
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
             SDRAM_MODEREG_CAS_LATENCY_3           |\
             SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

    /* Step 6: Set the refresh rate counter */
    /* Set the device refresh rate */
    HAL_SDRAM_ProgramRefreshRate(&sdramHandle, RefreshCount);
}

static void LowLevelInit(SDRAM_HandleTypeDef *hsdram)
{
    static DMA_HandleTypeDef dma_handle;
    GPIO_InitTypeDef gpio_init_structure;

    if (hsdram != (SDRAM_HandleTypeDef  *)NULL) {
        /* Enable FMC clock */
        __HAL_RCC_FMC_CLK_ENABLE();

        /* Enable GPIOs clock */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
#ifdef BOARD_DISCO469
        __HAL_RCC_GPIOI_CLK_ENABLE();
#endif

        /* Common GPIO configuration */
        gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
        gpio_init_structure.Pull      = GPIO_PULLUP;
        gpio_init_structure.Speed     = GPIO_SPEED_FAST;
        gpio_init_structure.Alternate = GPIO_AF12_FMC;

        /* GPIOC configuration : PC0 is SDNWE */
        gpio_init_structure.Pin   = GPIO_PIN_0;
        HAL_GPIO_Init(GPIOC, &gpio_init_structure);

        /* GPIOD configuration */
        gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 |\
                                    GPIO_PIN_14 | GPIO_PIN_15;
        HAL_GPIO_Init(GPIOD, &gpio_init_structure);

        /* GPIOE configuration */
        gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                                    GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                                    GPIO_PIN_15;
        HAL_GPIO_Init(GPIOE, &gpio_init_structure);

        /* GPIOF configuration */
        gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                                    GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                                    GPIO_PIN_15;
        HAL_GPIO_Init(GPIOF, &gpio_init_structure);

        /* GPIOG configuration */
        gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4| GPIO_PIN_5 | GPIO_PIN_8 |\
                                    GPIO_PIN_15;
        HAL_GPIO_Init(GPIOG, &gpio_init_structure);

        /* GPIOH configuration */
        gpio_init_structure.Pin   = GPIO_PIN_2 | GPIO_PIN_3;
        HAL_GPIO_Init(GPIOH, &gpio_init_structure);

#ifdef SDRAM_32BIT_ACCESS
        /* GPIOH configuration */
        gpio_init_structure.Pin   = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |\
                                    GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        HAL_GPIO_Init(GPIOH, &gpio_init_structure);

        /* GPIOI configuration */
        gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |\
                                    GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
        HAL_GPIO_Init(GPIOI, &gpio_init_structure);
#else
#ifdef BOARD_DISCO469
        // Force unused pins status in 16bit acces mode
        gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
        gpio_init_structure.Pull = GPIO_PULLUP;
        gpio_init_structure.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |\
                                    GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
        HAL_GPIO_Init(GPIOI, &gpio_init_structure);
        gpio_init_structure.Pin   = GPIO_PIN_4 | GPIO_PIN_5;
        gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
        gpio_init_structure.Pull      = GPIO_PULLDOWN;
        HAL_GPIO_Init(GPIOI, &gpio_init_structure);
        HAL_GPIO_WritePin(GPIOI, GPIO_PIN_4, 0);
        HAL_GPIO_WritePin(GPIOI, GPIO_PIN_5, 0);
#endif // BOARD_DISCO469
#endif // SDRAM_32BIT_ACCESS

#ifdef ENABLE_SDRAM_DMA
        __DMAx_CLK_ENABLE();
        dma_handle.Init.Channel             = SDRAM_DMAx_CHANNEL;
        dma_handle.Init.Direction           = DMA_MEMORY_TO_MEMORY;
        dma_handle.Init.PeriphInc           = DMA_PINC_ENABLE;
        dma_handle.Init.MemInc              = DMA_MINC_ENABLE;
        dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        dma_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
        dma_handle.Init.Mode                = DMA_NORMAL;
        dma_handle.Init.Priority            = DMA_PRIORITY_HIGH;
        dma_handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        dma_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
        dma_handle.Init.MemBurst            = DMA_MBURST_SINGLE;
        dma_handle.Init.PeriphBurst         = DMA_PBURST_SINGLE;

        dma_handle.Instance = SDRAM_DMAx_STREAM;
        __HAL_LINKDMA(hsdram, hdma, dma_handle);
        HAL_DMA_DeInit(&dma_handle);
        HAL_DMA_Init(&dma_handle);
        HAL_NVIC_SetPriority(SDRAM_DMAx_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(SDRAM_DMAx_IRQn);
#endif
    }
}

uint8_t STAR_SDRAM_Init(void)
{
    uint8_t sdramstatus = SDRAM_ERROR;
    FMC_SDRAM_TimingTypeDef Timing;

    /* SDRAM device configuration */
    sdramHandle.Instance = FMC_SDRAM_DEVICE;

    /* Timing configuration for 90 MHz as SD clock frequency (System clock is up to 180 MHz) */
    Timing.LoadToActiveDelay    = 2;
    Timing.ExitSelfRefreshDelay = 7;
    Timing.SelfRefreshTime      = 4;
    Timing.RowCycleDelay        = 7;
    Timing.WriteRecoveryTime    = 2;
    Timing.RPDelay              = 2;
    Timing.RCDDelay             = 2;

    sdramHandle.Init.SDBank             = FMC_SDRAM_BANK1;
    sdramHandle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
    sdramHandle.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
    sdramHandle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    sdramHandle.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
    sdramHandle.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
    sdramHandle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;
#ifdef SDRAM_32BIT_ACCESS
    sdramHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_11;
    sdramHandle.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
    sdramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
#else
    sdramHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
    sdramHandle.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_16;
    sdramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
#endif

    LowLevelInit(&sdramHandle);

    if (HAL_SDRAM_Init(&sdramHandle, &Timing) != HAL_OK)
        sdramstatus = SDRAM_ERROR;
    else
        sdramstatus = SDRAM_OK;

    InitSeq(REFRESH_COUNT);

    return sdramstatus;
}

// Test
#define TEST_BUF_SIZE   ((uint32_t)0x0100)
#define TEST_ADDR       ((uint32_t)0x600000)
uint8_t STAR_SDRAM_Test(void)
{
    int8_t res = SDRAM_OK;
    uint32_t buf[TEST_BUF_SIZE];
    uint32_t buf2[TEST_BUF_SIZE];
    uint32_t idx = 0;
    volatile uint32_t status = 0;

    for (idx = 0; idx < TEST_BUF_SIZE; idx++)
        buf[idx] = idx + 0xA244250F;

    for (idx = 0; idx < TEST_BUF_SIZE; idx++)
        *(volatile uint32_t*)(SDRAM_DEVICE_ADDR + TEST_ADDR + 4*idx) = buf[idx];

    for (idx = 0; idx < TEST_BUF_SIZE; idx++)
        buf2[idx] = *(volatile uint32_t*)(SDRAM_DEVICE_ADDR + TEST_ADDR + 4*idx);

    for (idx = 0; (idx < TEST_BUF_SIZE) && (status == 0); idx++)
        if (buf2[idx] != buf[idx])
            status++;

    if (status)
        res = SDRAM_ERROR;

    return res;
}
