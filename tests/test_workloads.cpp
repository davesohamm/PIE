#include "pie/workloads/workload.h"
#include <gtest/gtest.h>

using namespace pie::workloads;

TEST(WorkloadTest, MatrixMultiplication) {
    auto workload = WorkloadFactory::Create("matmul");
    ASSERT_NE(workload, nullptr);
    
    std::map<std::string, int> params = {{"size", 64}};
    workload->Initialize(params);
    workload->Execute();
    
    EXPECT_TRUE(workload->Verify());
    
    workload->Cleanup();
}

TEST(WorkloadTest, VectorAddition) {
    auto workload = WorkloadFactory::Create("vector_add");
    ASSERT_NE(workload, nullptr);
    
    std::map<std::string, int> params = {{"size", 1000}};
    workload->Initialize(params);
    workload->Execute();
    
    EXPECT_TRUE(workload->Verify());
    
    workload->Cleanup();
}

TEST(WorkloadTest, WorkloadFactory) {
    auto workloads = WorkloadFactory::ListWorkloads();
    EXPECT_GT(workloads.size(), 0);
}

TEST(WorkloadTest, InvalidWorkload) {
    EXPECT_THROW(WorkloadFactory::Create("invalid"), std::runtime_error);
}

