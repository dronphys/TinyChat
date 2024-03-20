#include "client.h"




void CustomClient::PingServer()
{
	net::message<CustomMsgTypes> msg;
	msg.header.id = CustomMsgTypes::ServerPing;

	auto timeNow = std::chrono::system_clock::now();

	msg << timeNow;
	std::cout << "SendingMsg\n";
	Send(msg);
}

void CustomClient::MessageAll(char* data)
{
	
	char dataToSend[MSG_MAX_LENGHT] = "";
	std::memcpy(dataToSend, data, sizeof(dataToSend));
	net::message<CustomMsgTypes> msg;
	msg.header.id = CustomMsgTypes::MessageAll;
	msg << dataToSend;
	Send(msg);
}

void CustomClient::Run()
{
	bool quit = false;
	InitWindow();
	while (!glfwWindowShouldClose(window) && !quit)
	{
		GUI();
		UpdateMessages();
	}
}

void CustomClient::ShowChangeNicknameWindow()
{
	ImGuiWindowFlags window_flags = 0;
	ImGui::Begin("Change NickName", &show_change_nickname_window, window_flags);
	ImGui::Text("Change your nickname here");
	ImGui::InputText("New Nickname", NewNickname, IM_ARRAYSIZE(NewNickname));
	if (ImGui::Button("Confirm change"))
	{
		Message msg;
		msg.header.id = CustomMsgTypes::ChangeNickname;
		msg << NewNickname;
		Send(msg);
		show_change_nickname_window = false;
		
	}
	ImGui::End();
}

void CustomClient::GUI()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glfwPollEvents();
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(window, 1);
	}
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	
	ImGui::NewFrame();
	{
		if (show_change_nickname_window) { ShowChangeNicknameWindow(); }

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_MenuBar;
		//window_flags |= ImGuiWindowFlags_NoResize;
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(width, height));

		

		ImGui::Begin("TinyChat", nullptr, window_flags);
		{

			
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Menu"))
				{
					ImGui::MenuItem("Change nickname", NULL, &show_change_nickname_window);
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
			ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 1.0f, 550), false, window_flags);
			for (auto& msg : allMessages)
			{
				uint32_t id = msg.first;
				std::string& msgBody = msg.second;
				// Show nickname if there is one, otherwise show id;
				if (m_nicknames.find(id) != m_nicknames.end())
				{
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), m_nicknames[id].data());
				}
				else
				{
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), std::to_string(id).data());
				}

				ImGui::SameLine();
				ImGui::Text(msgBody.data());
				if (newMessageArrived && msg == allMessages.back())
				{
					ImGui::SetScrollHereY(1.0f);
					newMessageArrived = false;
				}
			}
			ImGui::EndChild();
		}


		static ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue|
			ImGuiInputTextFlags_AllowTabInput 
			| ImGuiInputTextFlags_CtrlEnterForNewLine;

		if (shouldFocus)
		{
			ImGui::SetKeyboardFocusHere(0);
			shouldFocus = false;
		}
		if (ImGui::InputTextMultiline("##source", MessageText, IM_ARRAYSIZE(MessageText),
			ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5),
			flags))
		{
			sendMessageButtonCallback();
			shouldFocus = true;
		}

		if (ImGui::Button("Send Message"))
		{
			sendMessageButtonCallback();
		}

		ImGui::End();
	}


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
}

void CustomClient::InitWindow()
{
	int SCR_WIDTH = 600;
	int SCR_HEIGHT = 800;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TinyChat", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void CustomClient::UpdateMessages()
{
	if (!IsConnected())
	{
		return;
	}
	if (!Incoming().empty())
	{
		auto msg = Incoming().pop_front().msg;
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerAccept:
		{
			std::cout << "Server has accepted Connection\n";
		}
		break;
		case CustomMsgTypes::ServerPing:
		{
			auto timeNow = std::chrono::system_clock::now();
			std::chrono::system_clock::time_point timeThen;
			msg >> timeThen;
			std::cout << "Ping: " <<
				std::chrono::duration<double>(timeNow - timeThen).count() << '\n';
		}
		break;
	
		case CustomMsgTypes::ServerMessage:
		{
			uint32_t recvMsgId;
			char data[MSG_MAX_LENGHT];
			msg >> recvMsgId;
			msg >> data;
			allMessages.push_back({recvMsgId, std::string(data) });
			newMessageArrived = true;
		}
		break;
		case CustomMsgTypes::ChangeNickname:
		{
			uint32_t recvMsgId;
			char nickname[NICK_MAX_LENGHT];
			msg >> recvMsgId;
			msg >> nickname;
			m_nicknames[recvMsgId] = std::string{ nickname };
		}
		break;
		}
	}
	

}

void CustomClient::sendMessageButtonCallback()
{
	// do nothing if message is empty
	if (MessageText[0] == '\0') { return; }

	MessageAll(MessageText);
	for (char& c : MessageText)
	{
		c = '\0';
	}
}

int InputCallback(ImGuiInputTextCallbackData* data) {

	if (data->EventKey == ImGuiKey_Enter)
	{
		// Perform the desired action here
		// For example, print the current text to the console
		std::cout << "Enter pressed: \n";
	}
	return 0;
}
