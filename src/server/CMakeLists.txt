set(SOURCES
		"server.cpp"
		"ServerDatabase.cpp"
		"ServerCardData.cpp"
		"Creature.cpp"
		"Spell.cpp"
		"Player.cpp"
		"Constraints.cpp"
		"PostGameData.cpp"
		# sockets
		"sockets/Server.cpp"
		"sockets/GameThread.cpp"
	)

set(SERVER_NAME "${PROJECT_NAME}_server")

add_executable(${SERVER_NAME} ${SOURCES})

target_link_libraries(${SERVER_NAME} ${COMMON_NAME} ${EXTERNAL_LIBRARIES} pthread X11)


