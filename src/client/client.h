#include "net.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


class CustomClient : public net::client_interface<CustomMsgTypes>
{
public:

	void PingServer();
	void MessageAll(char* data);
	void Run();
private:
	void GUI();
	void InitWindow();
	void UpdateMessages();
	void sendMessageButtonCallback();
	void ShowChangeNicknameWindow();
	GLFWwindow* window;

	//id and message text
	std::list<std::pair<uint32_t, std::string>> allMessages;
	std::map<uint32_t, std::string> m_nicknames;
	char MessageText[MSG_MAX_LENGHT] = "";
	char NewNickname[NICK_MAX_LENGHT]= "";
	bool shouldFocus = false;
	//needed for scrolling to the botton if new message arrived
	bool newMessageArrived = false;
	bool show_change_nickname_window = false;
};

static int InputCallback(ImGuiInputTextCallbackData* data);