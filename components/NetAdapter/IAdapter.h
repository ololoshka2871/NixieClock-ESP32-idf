#ifndef _IADAPTER_H_
#define _IADAPTER_H_

namespace nw {

struct IAdapter {
  virtual ~IAdapter() = default;
  virtual void start() = 0;
};

} // namespace nw

#endif /* _IADAPTER_H_ */
