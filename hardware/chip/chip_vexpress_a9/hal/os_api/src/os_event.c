#include "typedef.h"
#include "aos_common.h"
#include "aos_event.h"


os_event_handle_t _os_event_group_create(void)
{
    int ret = 0;
    os_event__t *event=NULL;
    //event = nos_malloc(sizeof(os_event__t));
    //NULL_PARA_CHECK(event,NULL);
    ret = krhino_event_dyn_create(&event,"aos-event",0);
    if(0 == ret)
    {
        event_debug_info("create event sucess,handle:%x\r\n",event);
        event->flags = 0;
        return event;
    }
    else
    {
        event_debug_error("create event fail\r\n");
        return NULL;
    }
}

void _os_event_group_delete(os_event_handle_t event_handle)
{
    int ret = 0;
    
    ret = krhino_event_dyn_del(event_handle);
    if(0 == ret)
    {
        event_debug_info("delete event sucess\r\n");
    }
    else
    {
        event_debug_error("delete event fail\r\n");
    }
}

int _os_event_set(os_event_handle_t event_handle,os_event_bits_t set_bits)
{
    NULL_PARA_CHECK(event_handle,os_false);
    int ret = 0;
    ret = krhino_event_set(event_handle, set_bits, RHINO_OR);

    if(0 == ret)
    {
        return os_true;
    }
    else
    {
        event_debug_error("os event set bits fail,reason:%d\r\n",ret);
        return os_false;
    }
}

int _os_event_clear(os_event_handle_t event_handle,os_event_bits_t set_bits)
{
    NULL_PARA_CHECK(event_handle,os_false);
    int ret = 0;
    ret = krhino_event_set(event_handle, ~(set_bits), RHINO_AND);

    if(0 == ret)
    {
        return os_true;
    }
    else
    {
        event_debug_error("os event set bits fail,reason:%d\r\n",ret);
        return os_false;
    }
}


//wait_option 0:wait one of wait_bits 1:wait for all bits set
int _os_event_wait_bits(os_event_handle_t event_handle,os_event_bits_t wait_bits,int wait_option,int clear_option,os_event_bits_t *read_flags,os_tick_type_t block_time)
{
    NULL_PARA_CHECK(event_handle,os_false);
    NULL_PARA_CHECK(read_flags,os_false);
    uint32_t actl_flags=0;
    u32_t wait_time = (u32_t)block_time;
    int ret = 0;
    if(wait_option>0)
        wait_option = 1;
    else
        wait_option = 0;

    if(clear_option>0)
        clear_option = 1;
    else
        clear_option = 0;
    uint8_t Opt = (wait_option << 1u) | clear_option;

    if(OS_WAIT_FOREVER == block_time)
    {
        ret = krhino_event_get(event_handle, wait_bits, Opt, &actl_flags, RHINO_WAIT_FOREVER);
    }
    else
    {
        ret = krhino_event_get(event_handle, wait_bits, Opt, &actl_flags, block_time);
    }

    if(0 != ret)
    {
        event_debug_error("os sem wait fail reason:%d\r\n",ret);
        return os_false;
    }
    *read_flags = actl_flags;
    return os_true;
}
