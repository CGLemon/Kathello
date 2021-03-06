#ifndef CPUBACKEND_H_INCLUDE
#define CPUBACKEND_H_INCLUDE
#include "Model.h"
#include "config.h"
#include "Blas.h"

class CPUbackend : public Model::NNpipe {
public:
    virtual void initialize(std::shared_ptr<Model::NNweights> weights);
    virtual void forward(const int boardsize,
                         const std::vector<float> &planes,
                         const std::vector<float> &binary,
                         std::vector<float> &output_pol,
                         std::vector<float> &output_sb,
                         std::vector<float> &output_os,
                         std::vector<float> &output_fs,
                         std::vector<float> &output_val);

    virtual void reload(std::shared_ptr<Model::NNweights> weights);
    virtual void release();
    virtual void destroy() {}
    virtual bool valid();

private:
    std::shared_ptr<Model::NNweights> m_weights{nullptr};

};

#endif
