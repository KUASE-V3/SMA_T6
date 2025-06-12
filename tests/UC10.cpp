#include <gtest/gtest.h>
#include "domain/vendingMachine.h"
#include "service/DistanceService.hpp"

using domain::VendingMachine;
using service::OtherVendingMachineInfo;
using service::DistanceService;

TEST(SystemTest, BroadcastAndFindNearestVendingMachine) {
    // 1. 현재 자판기(t1) 정보
    int t1_x = 0, t1_y = 0;
    VendingMachine t1("T1", t1_x, t1_y, "12345");

    // 2. 브로드캐스트로 응답받은 다른 자판기 정보(t2, t3)
    std::vector<OtherVendingMachineInfo> responses;
    responses.push_back({"T2", 10, 10, true}); // t2: (10,0)
    responses.push_back({"T3", 5, 5, true});  // t3: (5,5)

    // 3. 거리 계산 및 가장 가까운 자판기 찾기
    DistanceService distanceService;
    auto nearestOpt = distanceService.findNearestAvailableVendingMachine(t1_x, t1_y, responses);

    ASSERT_TRUE(nearestOpt.has_value());
    VendingMachine nearest = nearestOpt.value();

    // 4. 사용자 안내 메시지(예상 결과)
    EXPECT_EQ(nearest.getId(), "T3");
    EXPECT_EQ(nearest.getLocation(), std::make_pair(5, 5));
}