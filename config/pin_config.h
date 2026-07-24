#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// $[CMU]
// [CMU]$

// $[LFXO]
// [LFXO]$

// $[PRS.ASYNCH0]
// [PRS.ASYNCH0]$

// $[PRS.ASYNCH1]
// [PRS.ASYNCH1]$

// $[PRS.ASYNCH2]
// [PRS.ASYNCH2]$

// $[PRS.ASYNCH3]
// [PRS.ASYNCH3]$

// $[PRS.ASYNCH4]
// [PRS.ASYNCH4]$

// $[PRS.ASYNCH5]
// [PRS.ASYNCH5]$

// $[PRS.ASYNCH6]
// [PRS.ASYNCH6]$

// $[PRS.ASYNCH7]
// [PRS.ASYNCH7]$

// $[PRS.ASYNCH8]
// [PRS.ASYNCH8]$

// $[PRS.ASYNCH9]
// [PRS.ASYNCH9]$

// $[PRS.ASYNCH10]
// [PRS.ASYNCH10]$

// $[PRS.ASYNCH11]
// [PRS.ASYNCH11]$

// $[PRS.SYNCH0]
// [PRS.SYNCH0]$

// $[PRS.SYNCH1]
// [PRS.SYNCH1]$

// $[PRS.SYNCH2]
// [PRS.SYNCH2]$

// $[PRS.SYNCH3]
// [PRS.SYNCH3]$

// $[GPIO]
// [GPIO]$

// $[TIMER0]
// [TIMER0]$

// $[TIMER1]
// [TIMER1]$

// $[TIMER2]
// [TIMER2]$

// $[TIMER3]
// [TIMER3]$

// $[TIMER4]
// [TIMER4]$

// $[USART0]
// USART0 CLK on PC02
#ifndef USART0_CLK_PORT                         
#define USART0_CLK_PORT                          SL_GPIO_PORT_C
#endif
#ifndef USART0_CLK_PIN                          
#define USART0_CLK_PIN                           2
#endif

// USART0 CS on PC03
#ifndef USART0_CS_PORT                          
#define USART0_CS_PORT                           SL_GPIO_PORT_C
#endif
#ifndef USART0_CS_PIN                           
#define USART0_CS_PIN                            3
#endif

// USART0 RX on PC01
#ifndef USART0_RX_PORT                          
#define USART0_RX_PORT                           SL_GPIO_PORT_C
#endif
#ifndef USART0_RX_PIN                           
#define USART0_RX_PIN                            1
#endif

// USART0 TX on PC00
#ifndef USART0_TX_PORT                          
#define USART0_TX_PORT                           SL_GPIO_PORT_C
#endif
#ifndef USART0_TX_PIN                           
#define USART0_TX_PIN                            0
#endif

// [USART0]$

// $[USART1]
// [USART1]$

// $[I2C1]
// [I2C1]$

// $[PDM]
// [PDM]$

// $[LETIMER0]
// [LETIMER0]$

// $[IADC0]
// [IADC0]$

// $[I2C0]
// [I2C0]$

// $[EUART0]
// EUART0 RX on PA05
#ifndef EUART0_RX_PORT                          
#define EUART0_RX_PORT                           SL_GPIO_PORT_A
#endif
#ifndef EUART0_RX_PIN                           
#define EUART0_RX_PIN                            5
#endif

// EUART0 TX on PA06
#ifndef EUART0_TX_PORT                          
#define EUART0_TX_PORT                           SL_GPIO_PORT_A
#endif
#ifndef EUART0_TX_PIN                           
#define EUART0_TX_PIN                            6
#endif

// [EUART0]$

// $[PTI]
// [PTI]$

// $[MODEM]
// [MODEM]$

// $[CUSTOM_PIN_NAME]
#ifndef _PORT                                   
#define _PORT                                    SL_GPIO_PORT_A
#endif
#ifndef _PIN                                    
#define _PIN                                     0
#endif

#ifndef EN_BTM_PORT                             
#define EN_BTM_PORT                              SL_GPIO_PORT_A
#endif
#ifndef EN_BTM_PIN                              
#define EN_BTM_PIN                               4
#endif

#ifndef PRESS_BTN_DETECT_PORT                   
#define PRESS_BTN_DETECT_PORT                    SL_GPIO_PORT_B
#endif
#ifndef PRESS_BTN_DETECT_PIN                    
#define PRESS_BTN_DETECT_PIN                     1
#endif

#ifndef POWER_LATCH_PORT                        
#define POWER_LATCH_PORT                         SL_GPIO_PORT_B
#endif
#ifndef POWER_LATCH_PIN                         
#define POWER_LATCH_PIN                          2
#endif


#ifndef ACT_LED_PORT                            
#define ACT_LED_PORT                             SL_GPIO_PORT_C
#endif
#ifndef ACT_LED_PIN                             
#define ACT_LED_PIN                              6
#endif

#ifndef LOW_BATT_SIGNAL_PORT                    
#define LOW_BATT_SIGNAL_PORT                     SL_GPIO_PORT_D
#endif
#ifndef LOW_BATT_SIGNAL_PIN                     
#define LOW_BATT_SIGNAL_PIN                      3
#endif

// [CUSTOM_PIN_NAME]$

#endif // PIN_CONFIG_H

