#include "player.hpp"
#include <iostream>
#include <unistd.h>

int main(void)
{
    // E.G. DATA
    std::stringstream ss;
    std::ifstream t("../video_samples/sample.ts");
    ss << t.rdbuf();
    
    Player player;
    player.AppendData(ss.str().c_str(), ss.tellp());

    sleep(1);
    player.Play();
    sleep(1);
    player.Pause();
    sleep(1);
    player.Play();

    getchar();

    player.Stop();

    LOG(LOG_INFO, "exiting...");

    return 0;
}
