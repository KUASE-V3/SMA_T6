#include <gtest/gtest.h>
#include "person.h"

TEST(PersonTest, ConstructorAndGetters) {
    Person p("Bob", 25);
    EXPECT_EQ(p.getName(), "Bob");
    EXPECT_EQ(p.getAge(), 25);
}

TEST(PersonTest, DifferentPerson) {
    Person p("Charlie", 40);
    EXPECT_EQ(p.getName(), "Charlie");
    EXPECT_EQ(p.getAge(), 40);
}
