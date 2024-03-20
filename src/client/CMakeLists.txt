set(CLIENT_APP TinyClient)
add_executable(${CLIENT_APP} 
    main.cpp
    client.cpp
    clientApplication.cpp
    userInterface.cpp)
    
target_link_libraries(${CLIENT_APP} PRIVATE networking)

target_link_libraries(${CLIENT_APP} PRIVATE imgui)
target_link_libraries(${CLIENT_APP} PRIVATE glad)
target_link_libraries(${CLIENT_APP} PRIVATE glfw)