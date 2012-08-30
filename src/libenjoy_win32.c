/**********************************************
**    This file is part of libenjoy
**    https://github.com/Tasssadar/libenjoy
**
**    See README and COPYING
***********************************************/

// This file is mostly copypaste from SDL
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <mmsystem.h>

// we dont want utf16 strings
#undef TEXT
#define TEXT(b) b

#include <regstr.h>

//#include <dinput.h> <-- mingw does not like this header.
WINMMAPI MMRESULT WINAPI joyConfigChanged( DWORD dwFlags );

#include "libenjoy.h"
#include "libenjoy_win32.h"
#include "libenjoy_p.h"

struct libenjoy_os_ctx *libenjoy_init_private(void)
{
    libenjoy_os_ctx *os = (libenjoy_os_ctx*)malloc(sizeof(libenjoy_os_ctx));
    os->known_devs = (libenjoy_known_info**)calloc(1, sizeof(libenjoy_known_info*));
    return os;
}

void libenjoy_close_private(libenjoy_os_ctx *os)
{
    if(os->known_devs)
    {
        libenjoy_known_info **i = os->known_devs;
        for(; *i != NULL; ++i)
            free(*i);
        free(os->known_devs);
    }
    free(os);
}

int libenjoy_get_joy_name(int index, const char *szRegKey, char *res)
{
    HKEY hTopKey;
    HKEY hKey;
    DWORD regsize;
    LONG regresult;
    char regkey[256];
    char regvalue[256];
    char regname[256];

    sprintf(regkey, "%s\\%s\\%s", REGSTR_PATH_JOYCONFIG, szRegKey, REGSTR_KEY_JOYCURR);
    hTopKey = HKEY_LOCAL_MACHINE;
    regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
    
    if (regresult != ERROR_SUCCESS)
    {
        hTopKey = HKEY_CURRENT_USER;
        regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
    }

    if (regresult != ERROR_SUCCESS)
        return -1;

    /* find the registry key name for the joystick's properties */
    regsize = sizeof(regname);
    sprintf(regvalue, "Joystick%d%s", index + 1, REGSTR_VAL_JOYOEMNAME);
    regresult = RegQueryValueExA(hKey, regvalue, 0, 0, (LPBYTE) regname, &regsize);
    RegCloseKey(hKey);

    if (regresult != ERROR_SUCCESS)
        return -1;

    /* open that registry key */
    sprintf(regkey, "%s\\%s", REGSTR_PATH_JOYOEM, regname);
    regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
    if (regresult != ERROR_SUCCESS)
        return -1;

    /* find the size for the OEM name text */
    regsize = sizeof(regvalue);
    regresult = RegQueryValueExA(hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, NULL, &regsize);
    if (regresult == ERROR_SUCCESS)
    {
        if(regsize >= 256) 
            return -1;

        regresult = RegQueryValueExA(hKey,REGSTR_VAL_JOYOEMNAME, 0, 0,
                                         (LPBYTE) res, &regsize);
    }
    RegCloseKey(hKey);
    return 0;
}

void libenjoy_enumerate(libenjoy_context *ctx)
{
    int i;
    int maxdevs = joyGetNumDevs();
    int numdevs = 0;
    JOYINFOEX joyinfo;
    JOYCAPSA joycaps;
    MMRESULT result;
    char name[256] = { 0 };
    uint32_t *existing_ids = libenjoy_create_existing_ids(ctx);
    int existing_count = ctx->info_list.count;

    // Without this, you have to replug joystick into the same USB port,
    // but when it is enabled, joystick reads return nonsense
    // FIXME: joyConfigChanged(0) causes WTF crashes in joyGetPosEx().
    // Sometimes.
    /*
    {
        char canRefresh = 1;
        for(i = 0; canRefresh == 1 && i < ctx->joy_list.count; ++i)
            if(ctx->joy_list.list[i]->os != NULL)
                canRefresh = 0;

        if(canRefresh == 1)
            joyConfigChanged(0);
    }*/

    for (i = JOYSTICKID1; i < maxdevs && numdevs < LIBENJOY_MAX_JOYSTICK; ++i)
    {
        joyinfo.dwSize = sizeof(joyinfo);
        joyinfo.dwFlags = JOY_RETURNALL;

        result = joyGetPosEx(i, &joyinfo);
        if (result != JOYERR_NOERROR)
            continue;

        result = joyGetDevCapsA(i, &joycaps, sizeof(joycaps));
        if (result == JOYERR_NOERROR)
        {
            uint32_t guid = (joycaps.wMid << 16) | joycaps.wPid;
            libenjoy_known_info *inf = libenjoy_get_known_dev(ctx->os, guid);
            if(!inf)
                inf = libenjoy_add_known_dev(ctx->os, guid, i);

            if(libenjoy_joy_info_created(ctx, inf->id) == 0)
            {
                libenjoy_joy_info *joy_inf = (libenjoy_joy_info*)malloc(sizeof(libenjoy_joy_info));
                joy_inf->id = inf->id;
                joy_inf->opened = 0;

                // Update id
                inf->sys_id = i;

                libenjoy_get_joy_name(i, joycaps.szRegKey, name);
                joy_inf->name = (char*)calloc(strlen(name)+1, sizeof(char));
                strcpy(joy_inf->name, name);

                libenjoy_add_joy_info(ctx, joy_inf);
            }
            // yeah, we are screwed. Two devices with same dev_id, just ignore the later one
            else if(libenjoy_set_id_exists(inf->id, existing_ids, existing_count) == 0)
                inf->sys_id = i;
        }   
    }
    
    // remove no longer existing joysticks
    for(i = 0; existing_ids && i < existing_count; ++i)
    {
        if(existing_ids[i] == UINT_MAX)
            continue;
        libenjoy_destroy_joy_info(ctx, existing_ids[i]);
    }
    free(existing_ids);
}

libenjoy_known_info *libenjoy_get_known_dev(libenjoy_os_ctx *octx, uint32_t guid)
{
    libenjoy_known_info **i = octx->known_devs;
    for(; *i != NULL; ++i)
    {
        if((*i)->guid == guid)
            return *i;
    }
    return NULL;
}

libenjoy_known_info *libenjoy_get_known_dev_by_id(libenjoy_os_ctx *octx, uint32_t id)
{
    libenjoy_known_info **i = octx->known_devs;
    for(; *i != NULL; ++i)
    {
        if((*i)->id == id)
            return *i;
    }
    return NULL;
}

libenjoy_known_info *libenjoy_add_known_dev(libenjoy_os_ctx *octx, uint32_t guid, UINT sys_id)
{
    uint32_t count = 2;
    libenjoy_known_info **i = octx->known_devs;
    libenjoy_known_info *inf = (libenjoy_known_info*)malloc(sizeof(libenjoy_known_info));

    for(; *i != NULL; ++i)
        ++count;

    octx->known_devs = (libenjoy_known_info**)realloc(octx->known_devs, sizeof(libenjoy_known_info*)*count);
    count -= 2;

    inf->guid = guid;
    inf->sys_id = sys_id;
    inf->id = libenjoy_get_new_joyid();

    octx->known_devs[count++] = inf;
    octx->known_devs[count] = NULL;
    return inf;
}

int libenjoy_set_id_exists(uint32_t id, uint32_t *list, int size)
{
    int i;
    for(i = 0; list && i < size; ++i)
    {
        if(list[i] == id)
        {
            list[i] = UINT_MAX;
            return 0;
        }
    }
    return -1;
}

uint32_t *libenjoy_create_existing_ids(libenjoy_context *ctx)
{
    uint32_t *res = NULL;
    uint32_t i;

    if(ctx->info_list.list == 0)
        return res;

    res = (uint32_t*)calloc(ctx->info_list.count, sizeof(uint32_t));
    for(i = 0; i < ctx->info_list.count; ++i)
        res[i] = ctx->info_list.list[i]->id;
    return res;
}

libenjoy_os_specific *libenjoy_open_os_specific(libenjoy_context *ctx, uint32_t id)
{
    libenjoy_known_info *inf = libenjoy_get_known_dev_by_id(ctx->os, id);
    libenjoy_os_specific *os;
    MMRESULT result;
    JOYCAPS joycaps;
    int axis_min[AXES_COUNT];
    int axis_max[AXES_COUNT];
    int caps_flags[AXES_COUNT - 2] = { JOYCAPS_HASZ, JOYCAPS_HASR, JOYCAPS_HASU, JOYCAPS_HASV };
    int i;

    if(!inf)
        return NULL;

    result = joyGetDevCaps(inf->sys_id, &joycaps, sizeof(joycaps));
    if(result != JOYERR_NOERROR)
        return NULL;

    axis_min[0] = joycaps.wXmin;
    axis_max[0] = joycaps.wXmax;
    axis_min[1] = joycaps.wYmin;
    axis_max[1] = joycaps.wYmax;
    axis_min[2] = joycaps.wZmin;
    axis_max[2] = joycaps.wZmax;
    axis_min[3] = joycaps.wRmin;
    axis_max[3] = joycaps.wRmax;
    axis_min[4] = joycaps.wUmin;
    axis_max[4] = joycaps.wUmax;
    axis_min[5] = joycaps.wVmin;
    axis_max[5] = joycaps.wVmax;

    os = (libenjoy_os_specific*)malloc(sizeof(libenjoy_os_specific));
    os->sys_id = inf->sys_id;

    for (i = 0; i < AXES_HATS_COUNT; ++i)
    {
        os->axes[i] = 0;
        
        if(i >= AXES_COUNT)
            continue;

        if ((i < 2) || (joycaps.wCaps & caps_flags[i - 2]))
        {
            os->axes_offset[i] = AXIS_MIN - axis_min[i];
            os->axes_scale[i] = (float) (AXIS_MAX - AXIS_MIN) / (axis_max[i] - axis_min[i]);
        }
        else
        {
            os->axes_offset[i] = 0;
            os->axes_scale[i] = 1.0f;
        }
    }

    for (i = 0; i < BUTTONS_COUNT; ++i)
        os->buttons[i] = 0;
    return os;
}

void libenjoy_set_parts_count(libenjoy_joystick *joy)
{
    MMRESULT result;
    JOYCAPS joycaps;
    libenjoy_known_info *inf = libenjoy_get_known_dev_by_id(joy->ctx->os, joy->id);

    if(!inf)
        goto failed;

    result = joyGetDevCaps(inf->sys_id, &joycaps, sizeof(joycaps));
    if(result != JOYERR_NOERROR)
        goto failed;

    joy->num_buttons =  joycaps.wNumButtons;
    joy->num_axes = joycaps.wNumAxes;

    if(joycaps.wCaps & JOYCAPS_HASPOV)
        joy->num_axes += 2;

    return;

failed:
    joy->num_axes = 0;
    joy->num_buttons = 0;
}

void libenjoy_close_os_specific(libenjoy_os_specific *os)
{
    free(os);
}

void libenjoy_poll_priv(libenjoy_context *ctx)
{
    MMRESULT result;
    DWORD pos[AXES_COUNT];
    const DWORD flags[AXES_COUNT] = { JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ,
        JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
    };
    libenjoy_joystick *joy;
    libenjoy_os_specific *os;
    libenjoy_event *ev;
    libenjoy_joystick_list *jlist = &ctx->joy_list;
    uint32_t i;
    uint32_t j;
    int value, change;

    for(j = 0; j < jlist->count; ++j)
    {
        JOYINFOEX joyinfo;

        joy = jlist->list[j];
        if(joy->valid == 0)
            continue;

        os = joy->os;
        
        joyinfo.dwSize = sizeof(joyinfo);
        joyinfo.dwFlags = JOY_RETURNALL;
        
        result = joyGetPosEx(os->sys_id, &joyinfo);
        if(result != JOYERR_NOERROR)
            continue;

        pos[0] = joyinfo.dwXpos;
        pos[1] = joyinfo.dwYpos;
        pos[2] = joyinfo.dwZpos;
        pos[3] = joyinfo.dwRpos;
        pos[4] = joyinfo.dwUpos;
        pos[5] = joyinfo.dwVpos;

        for (i = 0; i < (joy->num_axes-HATS_COUNT); ++i)
        {
            if (!(joyinfo.dwFlags & flags[i]))
                continue;
            
            value = (int) (((float)pos[i] + os->axes_offset[i]) * os->axes_scale[i]);

            change = (value - os->axes[i]);
            if ((change > -JOY_AXIS_THRESHOLD) && (change < JOY_AXIS_THRESHOLD))
                continue;
            
            os->axes[i] = value;

            ev = libenjoy_buff_get_for_write(ctx);
            ev->joy_id = joy->id;
            ev->type = LIBENJOY_EV_AXIS;
            ev->part_id = i;
            ev->data = value;

            libenjoy_buff_push(ctx);
        }

         /* joystick button events */
        if (joyinfo.dwFlags & JOY_RETURNBUTTONS)
        {   
            for (i = 0; i < joy->num_buttons; ++i)
            {
                int pressed = (joyinfo.dwButtons & (1 << i));
                if((pressed != 0 && os->buttons[i] == 0) || (pressed == 0 && os->buttons[i] != 0))
                {
                    os->buttons[i] = (pressed == 0) ? 0 : 1;

                    ev = libenjoy_buff_get_for_write(ctx);
                    ev->joy_id = joy->id;
                    ev->type = LIBENJOY_EV_BUTTON;
                    ev->part_id = i;
                    ev->data = (pressed != 0) ? 1 : 0;

                    libenjoy_buff_push(ctx);
                }
            }
        }

        /* joystick hat events - translate to axes events */
        if(joyinfo.dwFlags & JOY_RETURNPOV)
        {
            int16_t ax1 = 0, ax2 = 0;
            libenjoy_translate_pov(joyinfo.dwPOV, &ax1, &ax2);
            
            libenjoy_send_hat_event(joy, joy->num_axes-HATS_COUNT, ax1);
            libenjoy_send_hat_event(joy, joy->num_axes-HATS_COUNT+1, ax2);
        }
    }
}

void libenjoy_translate_pov(DWORD pov, int16_t *ax1,  int16_t *ax2)
{
    if(pov == JOY_POVCENTERED)
        return;

    if(pov > JOY_POVLEFT || pov < JOY_POVRIGHT)
        *ax2 = AXIS_MIN;
    else if(pov > JOY_POVRIGHT && pov < JOY_POVLEFT)
        *ax2 = AXIS_MAX;

    if(pov > JOY_POVFORWARD && pov < JOY_POVBACKWARD)
        *ax1 = AXIS_MIN;
    else if(pov > JOY_POVBACKWARD)
        *ax1 = AXIS_MAX;
}

void libenjoy_send_hat_event(libenjoy_joystick *joy, int idx, int16_t val)
{
    libenjoy_event *ev;

    if(joy->os->axes[idx] == val)
        return;

    joy->os->axes[idx] = val;

    ev = libenjoy_buff_get_for_write(joy->ctx);
    ev->joy_id = joy->id;
    ev->type = LIBENJOY_EV_AXIS;
    ev->part_id = idx;
    ev->data = val;
    libenjoy_buff_push(joy->ctx);
}
