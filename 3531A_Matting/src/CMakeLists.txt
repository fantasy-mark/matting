cmake_minimum_required(VERSION 3.9)
project(MATTING)


#-----------------------------------------------------------------------------
ADD_CUSTOM_TARGET(MATTING ALL
        COMMENT "Create Application"
        )

ADD_CUSTOM_COMMAND(TARGET MATTING
        POST_BUILD #PRE_BUILD PRE_LINK POST_BUILD
        COMMAND sh /home/mark/3531A抠图/3531A_Matting/src/build.sh
        )
#-----------------------------------------------------------------------------
