
/** \file
 * Header file for the TunerStudio command handler
 * The command handler manages all the inputs FROM TS which are issued when a command button is clicked by the user
 */

static constexpr uint16_t TS_CMD_TEST_DSBL    = 256;
static constexpr uint16_t TS_CMD_TEST_ENBL    = 257;

static constexpr uint16_t TS_CMD_INJ1_ON      = 513;
static constexpr uint16_t TS_CMD_INJ1_OFF     = 514;
static constexpr uint16_t TS_CMD_INJ1_PULSED  = 515;
static constexpr uint16_t TS_CMD_INJ2_ON      = 516;
static constexpr uint16_t TS_CMD_INJ2_OFF     = 517;
static constexpr uint16_t TS_CMD_INJ2_PULSED  = 518;
static constexpr uint16_t TS_CMD_INJ3_ON      = 519;
static constexpr uint16_t TS_CMD_INJ3_OFF     = 520;
static constexpr uint16_t TS_CMD_INJ3_PULSED  = 521;
static constexpr uint16_t TS_CMD_INJ4_ON      = 522;
static constexpr uint16_t TS_CMD_INJ4_OFF     = 523;
static constexpr uint16_t TS_CMD_INJ4_PULSED  = 524;
static constexpr uint16_t TS_CMD_INJ5_ON      = 525;
static constexpr uint16_t TS_CMD_INJ5_OFF     = 526;
static constexpr uint16_t TS_CMD_INJ5_PULSED  = 527;
static constexpr uint16_t TS_CMD_INJ6_ON      = 528;
static constexpr uint16_t TS_CMD_INJ6_OFF     = 529;
static constexpr uint16_t TS_CMD_INJ6_PULSED  = 530;
static constexpr uint16_t TS_CMD_INJ7_ON      = 531;
static constexpr uint16_t TS_CMD_INJ7_OFF     = 532;
static constexpr uint16_t TS_CMD_INJ7_PULSED  = 533;
static constexpr uint16_t TS_CMD_INJ8_ON      = 534;
static constexpr uint16_t TS_CMD_INJ8_OFF     = 535;
static constexpr uint16_t TS_CMD_INJ8_PULSED  = 536;

static constexpr uint16_t TS_CMD_IGN1_ON      = 769;
static constexpr uint16_t TS_CMD_IGN1_OFF     = 770;
static constexpr uint16_t TS_CMD_IGN1_PULSED  = 771;
static constexpr uint16_t TS_CMD_IGN2_ON      = 772;
static constexpr uint16_t TS_CMD_IGN2_OFF     = 773;
static constexpr uint16_t TS_CMD_IGN2_PULSED  = 774;
static constexpr uint16_t TS_CMD_IGN3_ON      = 775;
static constexpr uint16_t TS_CMD_IGN3_OFF     = 776;
static constexpr uint16_t TS_CMD_IGN3_PULSED  = 777;
static constexpr uint16_t TS_CMD_IGN4_ON      = 778;
static constexpr uint16_t TS_CMD_IGN4_OFF     = 779;
static constexpr uint16_t TS_CMD_IGN4_PULSED  = 780;
static constexpr uint16_t TS_CMD_IGN5_ON      = 781;
static constexpr uint16_t TS_CMD_IGN5_OFF     = 782;
static constexpr uint16_t TS_CMD_IGN5_PULSED  = 783;
static constexpr uint16_t TS_CMD_IGN6_ON      = 784;
static constexpr uint16_t TS_CMD_IGN6_OFF     = 785;
static constexpr uint16_t TS_CMD_IGN6_PULSED  = 786;
static constexpr uint16_t TS_CMD_IGN7_ON      = 787;
static constexpr uint16_t TS_CMD_IGN7_OFF     = 788;
static constexpr uint16_t TS_CMD_IGN7_PULSED  = 789;
static constexpr uint16_t TS_CMD_IGN8_ON      = 790;
static constexpr uint16_t TS_CMD_IGN8_OFF     = 791;
static constexpr uint16_t TS_CMD_IGN8_PULSED  = 792;

static constexpr uint16_t TS_CMD_STM32_REBOOT     = 12800;
static constexpr uint16_t TS_CMD_STM32_BOOTLOADER = 12801;

static constexpr uint16_t TS_CMD_SD_FORMAT  = 13057;

static constexpr uint16_t TS_CMD_VSS_60KMH  = 39168; //0x99x00
static constexpr uint16_t TS_CMD_VSS_RATIO1 = 39169;
static constexpr uint16_t TS_CMD_VSS_RATIO2 = 39170;
static constexpr uint16_t TS_CMD_VSS_RATIO3 = 39171;
static constexpr uint16_t TS_CMD_VSS_RATIO4 = 39172;
static constexpr uint16_t TS_CMD_VSS_RATIO5 = 39173;
static constexpr uint16_t TS_CMD_VSS_RATIO6 = 39174;

/* the maximum id number is 65,535 */
bool TS_CommandButtonsHandler(uint16_t buttonCommand);
