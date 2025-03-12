#include <types.h>
#include <compiler.h>
#include <module.h>
#include <printk.h>

int
testmodule (void)
{
        printk ("Hello\n");
        return 0;
}

void
testinit (void)
{
        testmodule ();
}

MODULE_DECL test = {
        .name           = "Test",
        .description    = "just testtest",
        .init           = testinit,
        .delete         = NULL,
};
