#include "rubysdl2_internal.h"
#include <SDL_gamecontroller.h>

static VALUE cGameController;

typedef struct GameController {
    SDL_GameController* controller;
} GameController;

static void GameController_free(GameController* g)
{
    if (rubysdl2_is_active() && g->controller)
        SDL_GameControllerClose(g->controller);
    free(g);
}

static VALUE GameController_new(SDL_GameController* controller)
{
    GameController* g = ALLOC(GameController);
    g->controller = controller;
    return Data_Wrap_Struct(cGameController, 0, GameController_free, g);
}

DEFINE_WRAPPER(SDL_GameController, GameController, controller, cGameController,
               "SDL2::GameController");


static VALUE GameController_s_add_mapping(VALUE self, VALUE string)
{
    int ret = HANDLE_ERROR(SDL_GameControllerAddMapping(StringValueCStr(string)));
    return INT2NUM(ret);
}

static VALUE GameController_s_axis_name_of(VALUE self, VALUE axis)
{
    const char* name = SDL_GameControllerGetStringForAxis(NUM2INT(axis));
    if (!name) {
        SDL_SetError("Unknown axis %d", NUM2INT(axis));
        SDL_ERROR();
    }
    return utf8str_new_cstr(name);
}

static VALUE GameController_s_button_name_of(VALUE self, VALUE button)
{
    const char* name = SDL_GameControllerGetStringForButton(NUM2INT(button));
    if (!name) {
        SDL_SetError("Unknown axis %d", NUM2INT(button));
        SDL_ERROR();
    }
    return utf8str_new_cstr(name);
}

static VALUE GameController_s_axis_from_name(VALUE self, VALUE name)
{
    int axis = SDL_GameControllerGetAxisFromString(StringValueCStr(name));
    if (axis < 0) {
        SDL_SetError("Unknown axis name \"%s\"", StringValueCStr(name));
        SDL_ERROR();
    } 
    return INT2FIX(axis);
}

static VALUE GameController_s_button_from_name(VALUE self, VALUE name)
{
    int button = SDL_GameControllerGetButtonFromString(StringValueCStr(name));
    if (button < 0) {
        SDL_SetError("Unknown button name \"%s\"", StringValueCStr(name));
        SDL_ERROR();
    }
    return INT2FIX(button);
}

static VALUE GameController_s_device_names(VALUE self)
{
    int num_joysticks = SDL_NumJoysticks();
    int i;
    VALUE device_names = rb_ary_new2(num_joysticks);
    for (i=0; i<num_joysticks; ++i) {
        const char* name = SDL_GameControllerNameForIndex(i);
        if (name)
            rb_ary_push(device_names, utf8str_new_cstr(name));
        else
            rb_ary_push(device_names, Qnil);
    }
    return device_names;
}

static VALUE GameController_s_mapping_for(VALUE self, VALUE guid_string)
{
    SDL_JoystickGUID guid = SDL_JoystickGetGUIDFromString(StringValueCStr(guid_string));
    return utf8str_new_cstr(SDL_GameControllerMappingForGUID(guid));
}

static VALUE GameController_s_open(VALUE self, VALUE index)
{
    SDL_GameController* controller = SDL_GameControllerOpen(NUM2INT(index));
    if (!controller)
        SDL_ERROR();
    return GameController_new(controller);
}

static VALUE GameController_name(VALUE self)
{
    return utf8str_new_cstr(SDL_GameControllerName(Get_SDL_GameController(self)));
}

static VALUE GameController_attached_p(VALUE self)
{
    return INT2BOOL(SDL_GameControllerGetAttached(Get_SDL_GameController(self)));
}

static VALUE GameController_destroy(VALUE self)
{
    GameController* g = Get_GameController(self);
    if (g->controller)
        SDL_GameControllerClose(g->controller);
    g->controller = NULL;
    return Qnil;
}

static VALUE GameController_mapping(VALUE self)
{
    return utf8str_new_cstr(SDL_GameControllerMapping(Get_SDL_GameController(self)));
}

static VALUE GameController_axis(VALUE self, VALUE axis)
{
    return INT2FIX(SDL_GameControllerGetAxis(Get_SDL_GameController(self),
                                             NUM2INT(axis)));
}

static VALUE GameController_button_pressed_p(VALUE self, VALUE button)
{
    return INT2BOOL(SDL_GameControllerGetButton(Get_SDL_GameController(self),
                                                NUM2INT(button)));
}

void rubysdl2_init_gamecontorller(void)
{
    cGameController = rb_define_class_under(mSDL2, "GameController", rb_cObject);

    rb_define_singleton_method(cGameController, "add_mapping",
                               GameController_s_add_mapping, 1);
    rb_define_singleton_method(cGameController, "device_names",
                               GameController_s_device_names, 0);
    rb_define_singleton_method(cGameController, "axis_name_of",
                               GameController_s_axis_name_of, 1);
    rb_define_singleton_method(cGameController, "button_name_of",
                               GameController_s_button_name_of, 1);
    rb_define_singleton_method(cGameController, "mapping_for",
                               GameController_s_mapping_for, 1);
    rb_define_singleton_method(cGameController, "button_from_name",
                               GameController_s_button_from_name, 1);
    rb_define_singleton_method(cGameController, "axis_from_name",
                               GameController_s_axis_from_name, 1);
    rb_define_singleton_method(cGameController, "open", GameController_s_open, 1);
    rb_define_method(cGameController, "destroy?", GameController_destroy_p, 0);
    rb_define_method(cGameController, "destroy", GameController_destroy, 0);
    rb_define_method(cGameController, "name", GameController_name, 0);
    rb_define_method(cGameController, "attached?", GameController_attached_p, 0);
    rb_define_method(cGameController, "mapping", GameController_mapping, 0);
    rb_define_method(cGameController, "axis", GameController_axis, 1);
    rb_define_method(cGameController, "button_pressed?", GameController_button_pressed_p, 1);
#define DEFINE_CONTROLLER_AXIS_CONST(type) \
    rb_define_const(cGameController, "AXIS_" #type, INT2NUM(SDL_CONTROLLER_AXIS_##type))
    DEFINE_CONTROLLER_AXIS_CONST(INVALID);
    DEFINE_CONTROLLER_AXIS_CONST(LEFTX);
    DEFINE_CONTROLLER_AXIS_CONST(LEFTY);
    DEFINE_CONTROLLER_AXIS_CONST(RIGHTX);
    DEFINE_CONTROLLER_AXIS_CONST(RIGHTY);
    DEFINE_CONTROLLER_AXIS_CONST(TRIGGERLEFT);
    DEFINE_CONTROLLER_AXIS_CONST(TRIGGERRIGHT);
    DEFINE_CONTROLLER_AXIS_CONST(MAX);
#define DEFINE_CONTROLLER_BUTTON_CONST(type) \
    rb_define_const(cGameController, "BUTTON_" #type, INT2NUM(SDL_CONTROLLER_BUTTON_##type))
    DEFINE_CONTROLLER_BUTTON_CONST(INVALID);
    DEFINE_CONTROLLER_BUTTON_CONST(A);
    DEFINE_CONTROLLER_BUTTON_CONST(B);
    DEFINE_CONTROLLER_BUTTON_CONST(X);
    DEFINE_CONTROLLER_BUTTON_CONST(Y);
    DEFINE_CONTROLLER_BUTTON_CONST(BACK);
    DEFINE_CONTROLLER_BUTTON_CONST(GUIDE);
    DEFINE_CONTROLLER_BUTTON_CONST(START);
    DEFINE_CONTROLLER_BUTTON_CONST(LEFTSTICK);
    DEFINE_CONTROLLER_BUTTON_CONST(RIGHTSTICK);
    DEFINE_CONTROLLER_BUTTON_CONST(LEFTSHOULDER);
    DEFINE_CONTROLLER_BUTTON_CONST(RIGHTSHOULDER);
    DEFINE_CONTROLLER_BUTTON_CONST(DPAD_UP);
    DEFINE_CONTROLLER_BUTTON_CONST(DPAD_DOWN);
    DEFINE_CONTROLLER_BUTTON_CONST(DPAD_LEFT);
    DEFINE_CONTROLLER_BUTTON_CONST(DPAD_RIGHT);
    DEFINE_CONTROLLER_BUTTON_CONST(MAX);
}
