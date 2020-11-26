#include<Buffer/Buffer.h>
#include<gtest/gtest.h>
#include<string>

TEST(SETTING, test){
    EXPECT_EQ(1,1);
}

TEST(BUFFER, construct){
    Buffer buf;
    size_t size = buf.get_writeable_size();
    EXPECT_EQ(static_cast<size_t>(1024), size);
}

TEST(BUFFER, normal_append){
    Buffer buf;
    std::string  test_str = "test_string";
    buf.append(test_str);
    size_t size = static_cast<size_t>(test_str.size());
    EXPECT_EQ(size,buf.get_readable_size());
    EXPECT_EQ(size + buf.get_writeable_size(),1024);
    EXPECT_EQ(0,buf.get_prepared_size());
    buf.clear_all();
    EXPECT_EQ(0,buf.get_readable_size());
    EXPECT_EQ(1024,buf.get_writeable_size());
    EXPECT_EQ(0,buf.get_prepared_size());
}

TEST(BUFFER, expand){
    Buffer buf(16);
    EXPECT_EQ(16,buf.get_writeable_size());
    std::string test_str = "sjsajfjliasj;djfijasdijfdlsajdfialsjdifdaf";
    buf.append(test_str);
    EXPECT_EQ(1,buf.get_writeable_size());
    EXPECT_EQ(test_str.size(),buf.get_readable_size());
    EXPECT_EQ(0,buf.get_prepared_size());
}





int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

