#include "player.hpp"
#include <iostream>

int main(void)
{
    // E.G. DATA
    std::stringstream ss;
    std::ifstream t("../video_samples/sample.ts");
    ss << t.rdbuf();
    
    Player player;
    player.AppendData(ss.str().c_str(), ss.tellp());
    player.AppendData(ss.str().c_str(), ss.tellp());

    player.Play();
    
    player.Pause();
    sleep(2);

    player.Play();
    sleep(10);

    player.AppendData(ss.str().c_str(), ss.tellp());
    sleep(10);

    player.Stop();

    LOG("MAIN", LOG_INFO, "exiting...");

    return 0;
}