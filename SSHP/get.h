//////////////////////////////////////////////////////////
//
// ��ȡ��¼��֤��: "/api/get"
//   ������֤��ǰ��
//
//////////////////////////////////////////////////////////
#pragma once
#include "aops.h"
#include <atomic>
#include <random>

// ��¼��֤��
struct get_t
{
  http_server& server;
  // ��¼��֤��ͼƬ����
  static std::atomic_int file_count;
  // ����X,Y
  int rand_range(int x, int y)
  {
    unsigned int seed = (unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);
    std::uniform_int_distribution<int> range(x, y);
    return range(engine);
  }

  void operator()(request& req, response& res)
  {
    if (0 == file_count)
    {
      // ��һ����ͳ��һ�µ�¼��֤������
      std::filesystem::path path("./www/captcha/login/");
      for (auto& fe : std::filesystem::directory_iterator(path))
      {
        if (fe.is_directory())
          continue;
        file_count++;
      }
      // ��ͼ�뻬����������һ��
      if (file_count % 2)
      {
        spdlog::info("��¼��֤��ͼƬ��������ȷ,�������");
        return;
      }
      file_count.store(file_count.load() >> 1);
      spdlog::info("����֤������:{}", file_count);
    }

    // ���һ����֤��ͼƬ����
    int index = rand_range(1, file_count);
    auto path = fmt::format("./www/captcha/login/{}.jpg", index); // ��ͼ
    auto path1= fmt::format("./www/captcha/login/{}.png", index); // ����
    if (!std::filesystem::exists(path) || !std::filesystem::exists(path1))
    {
      res.set_status_and_content(status_type::not_found, "file not found");
      return;
    }
    uintmax_t jpgsize = std::filesystem::file_size(path);
    uintmax_t pngsize = std::filesystem::file_size(path1);
    std::string strsize = fmt::format("{0:0>6}{1:0>6}", jpgsize, pngsize);

    // ��Ӧͷ
    res.add_header("Access-Control-Allow-Credentials", "true");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
    res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.add_header("Access-Control-Expose-Headers", "gpcaptchaid,gpcaptchal");
    res.add_header("gpcaptchaid", "142980a3-6422-456f-8f49-3899e0ca473f");
    res.add_header("gpcaptchal", std::move(strsize));
    res.add_header("Set-Cookie", "gpcaptchaid=142980a3-6422-456f-8f49-3899e0ca473f; Path=/; HTTPOnly");

    // ��ȡ��֤��ͼƬ
    auto ifs_ptr = std::make_shared<std::ifstream>(path, std::ios_base::binary);
    std::stringstream file_buffer;
    file_buffer << ifs_ptr->rdbuf();
    auto ifs_ptr1 = std::make_shared<std::ifstream>(path1, std::ios_base::binary);
    file_buffer << ifs_ptr1->rdbuf();
    server.send_small_file_content(res, file_buffer, "text/plain");
  }
};

std::atomic_int get_t::file_count = 0;