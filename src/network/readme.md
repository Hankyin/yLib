# 网络模块
本模块实现了部分网络功能
1. 对tcp进行了基本的封装
2. 基于阻塞IO实现了HTTP客户端
3. 基于阻塞IO实现了HTTP服务端

## TCP的封装

TODO错误处理
    超时错误
    对方主动拒绝

## HTTP的实现

关于http消息长度的确定

对于http的请求返回结果要进行内容的长度校验主要有两种方式，二者互斥使用
1.客户端在http头(head)加Connection:keep-alive时，服务器的response是Transfer-Encoding:chunked的形式，通知页面数据是否接收完毕，例如长连接或者程序运行中可以动态的输出内容，例如一些运算比较复杂且需要用户及时的得到最新结果，那就采用chunked编码将内容分块输出。

2.除了如1所述之外的情况一般都是可以获取到Content-Length的。



在HTTP协议中，Content-Length用于描述HTTP消息实体的传输长度the transfer-length of the message-body。在HTTP协议中，消息实体长度和消息实体的传输长度是有区别，比如说gzip压缩下，消息实体长度是压缩前的长度，消息实体的传输长度是gzip压缩后的长度。

在具体的HTTP交互中，客户端是如何获取消息长度的呢，主要基于以下几个规则：

响应为1xx，204，304相应或者head请求，则直接忽视掉消息实体内容。
如果有Transfer-Encoding，则优先采用Transfer-Encoding里面的方法来找到对应的长度。比如说Chunked模式。
“如果head中有Content-Length，那么这个Content-Length既表示实体长度，又表示传输长度。如果实体长度和传输长度不相等（比如说设置了Transfer-Encoding），那么则不能设置Content-Length。如果设置了Transfer-Encoding，那么Content-Length将被忽视”。这句话翻译的优点饶，其实关键就一点：有了Transfer-Encoding，则不能有Content-Length。
Range传输。不关注，没详细看了：）
通过服务器关闭连接能确定消息的传输长度。（请求端不能通过关闭连接来指明请求消息体的结束，因为这样可以让服务器没有机会继续给予响应）。这种情况主要对应为短连接，即非keep-alive模式。
HTTP1.1必须支持chunk模式。因为当不确定消息长度的时候，可以通过chunk机制来处理这种情况。
在包含消息内容的header中，如果有content-length字段，那么该字段对应的值必须完全和消息主题里面的长度匹配。
“The entity-length of a message is the length of the message-body before any transfer-codings have been applied”
也就是有chunk就不能有content-length 。
其实后面几条几乎可以忽视，简单总结后如下：

1、Content-Length如果存在并且有效的话，则必须和消息内容的传输长度完全一致。（经过测试，如果过短则会截断，过长则会导致超时。）

2、如果存在Transfer-Encoding（重点是chunked），则在header中不能有Content-Length，有也会被忽视。

3、如果采用短连接，则直接可以通过服务器关闭连接来确定消息的传输长度。（这个很容易懂）

结合HTTP协议其他的特点，比如说Http1.1之前的不支持keep alive。那么可以得出以下结论：

1、在Http 1.0及之前版本中，content-length字段可有可无。

2、在http1.1及之后版本。如果是keep alive，则content-length和chunk必然是二选一。若是非keep alive，则和http1.0一样。content-length可有可无
————————————————
版权声明：本文为CSDN博主「superhosts」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/superhosts/article/details/8737434


## HTTP消息读取
每当有一个客户端连接服务器，服务器就会分配一个线程给他。然后双方进行HTTP报文的交互，可以进行多次。
1. 查找/r/n/r/n，找到后将首行和头部信息分割出来。 进行过长检测和超时处理。
2. 解析首行和头部。进行格式判断。
3. 路由解析，得到正确的Handler
4. 根据Handler创建req和resp对象，
5. 读取body，如果req有callback，则调用callback，没有就直接读取全部放在req的body中。
6. req读取完成，调用handler_func
7. handler_func可以查看req，需要构造resp
8. 向客户端发送resp，如果resp有callback，就调用callback，否则将body中的数据全部发出去。

错误处理采用异常处理
路由失败：抛出HTTPRouteException，返回404
格式错误：抛出HTTPFormatException，返回客户端错误
Content-Length解析失败：抛出其他异常，直接返回
# TODO
HTTP超时处理：
长期没有发送满足要求的HTTP报文直接认为超时，直接关闭socket

HTTP消息中的中文处理

