#include <types.h>
#include <compiler.h>
#include <timer.h>
#include <panic.h>
#include <printk.h>
#include <proc.h>
#include <x86/cpu.h>

#define MSEC2USEC	1000

Timer *systimer;

// TIMER *Timer PERCPU;
// EVENTTIMER *EventTimer PERCPU;

static Timer *
usingtimer (void)
{
  return systimer;
}

void
msleep (uint msec)
{
  uint usec = msec * MSEC2USEC;
  usleep (usec);
}

void
usleep (uint usec)
{
  Timer *timer = usingtimer ();
  ulong now, after;

  if (!timer)
    return;
  now = timer->read (timer);
  after = now + timer->usec2period (timer, usec);

  while (timer->read (timer) < after)
    ;
}

int
eventtimerirq (Irq *irq)
{
  Device      *dev = irq->device;
  EventTimer  *et  = dev->priv; 
  int         ret;

  ret = et->irqhandler (et, irq);

  schedule ();

  return ret;
}

int
probeevtimer (Device *dev)
{
  EventTimer *et = (EventTimer *)dev->priv;

  if (et->global) {
    ;
  } else {
    ;
  }
  return 0;
}

int
probetimer (Device *dev)
{
  Timer *tm = (Timer *)dev->priv;

  if (tm->global) {
    if (systimer) {
      KLOG ("systimer changed\n");
    }
    KLOG ("new systimer: %d\n", dev->name);
    systimer = tm;
    return 0;
  } else {
    return -1;
  }
}
