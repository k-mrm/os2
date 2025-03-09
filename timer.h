#ifndef _TIMER_H
#define _TIMER_H

#include <types.h>
#include <compiler.h>
#include <device.h>
#include <irq.h>

typedef struct Timer      Timer;
typedef struct EventTimer EventTimer;

struct Timer
{
        DEVICE_STRUCT;

        void    *priv;
        bool    global;
        ulong   (*usec2period) (Timer *tm, uint usec);
        ulong   (*read) (Timer *tm);
};

struct EventTimer
{
        DEVICE_STRUCT;

        void  *priv;
        bool  global;
        uint  (*getperiod) (EventTimer *et);
        void  (*setperiod) (EventTimer *et, uint ms);
        int   (*irqhandler) (EventTimer *et, Irq *irq);
};

int eventtimerirq (Irq *irq);

int probeevtimer (Device *dev);
int probetimer (Device *dev);
void msleep (uint msec);
void usleep (uint usec);

#endif  // _TIMER_H
