//
// Created by Tomas on 16. 2. 2021.
//

#ifndef ZOS_MACROS_H
#define ZOS_MACROS_H

#define e_return_on_error(element,message) \
if(element==ERROR_CODE){\
    printf(message);\
    return;\
}

#define e_return_value_on_error(element,message,value) \
if(element==ERROR_CODE){\
    printf(message);\
    return value;\
}

#define e_return_on_null(element,message) \
if(element==NULL){\
    printf(message);\
    return;\
}

#define e_return_value_on_null(element,message,value) \
if(element==NULL){\
    printf(message);\
    return value;\
}

#define e_return_on_condition(condition,message) \
if(condition){\
    printf(message);\
    return;\
}

#define e_return_value_on_condition(condition,message,value) \
if(condition){\
    printf(message);\
    return value;\
}

#endif //ZOS_MACROS_H
