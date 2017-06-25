// (c) Tobias Faller 2017

#ifndef __BUTTONS_P_H
#define __BUTTONS_P_H

/**
 * Callback function for timer 2 interrupt.
 *
 * @return true if the CPU should be woken up
 */
bool_t
buttons_on_timer2 (void);

#endif // !__BUTTONS_P_H
