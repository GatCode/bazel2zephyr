#include <zephyr.h>
#include <sys/printk.h>
#include "calculationMachine.hh"

void main(void) {
	while(1) {
		CalculationMachine machine = CalculationMachine();
		printk("Sum of 5 and 3 = %d\n", machine.sum(5, 3));
		k_sleep(K_MSEC(1000));
	}
}
