if(EXISTS "/home/ntg/github_projects/DogStory/build/game_server_tests_tests-b12d07c.cmake")
  include("/home/ntg/github_projects/DogStory/build/game_server_tests_tests-b12d07c.cmake")
else()
  add_test(game_server_tests_NOT_BUILT-b12d07c game_server_tests_NOT_BUILT-b12d07c)
endif()
