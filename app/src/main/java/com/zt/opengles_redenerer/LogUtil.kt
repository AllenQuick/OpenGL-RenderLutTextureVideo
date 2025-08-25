package com.zt.opengles_redenerer

import android.util.Log
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject
import java.util.Locale
import kotlin.collections.indexOfLast
import kotlin.collections.toTypedArray
import kotlin.jvm.java
import kotlin.text.isEmpty
import kotlin.text.split
import kotlin.text.startsWith
import kotlin.text.substring
import kotlin.text.uppercase

/**
 * @author 杨耿雷
 * @date 2024/9/30 13:39
 * @description Logging tool class
 * 使用[LogUtil.init]方法进行初始化，设置是否显示日志。
 * 使用[LogUtil.d]方法进行调试，仅输出debug级别的调试信息。可以通过Logcat标签进行过滤选择输出的信息。
 * 使用[LogUtil.e]方法进行调试，输出error级别的错误信息，以红色显示。需要认真分析并查看栈信息。
 * 使用[LogUtil.v]方法进行调试，输出任何消息，即verbose级别。平时使用Log.v("", "")。
 * 使用[LogUtil.i]方法进行调试，输出提示性的信息，即information级别。不会输出Log.v和Log.d的信息，但会显示i、w和e级别的信息。
 * 使用[LogUtil.w]方法进行调试，输出警告信息，即warning级别。需要注意优化Android代码，并会输出Log.e级别的信息。
 * 使用[LogUtil.wtf]方法进行调试，(what the fuck?)，形容正常情况下永远不会发生bug？
 * 使用[LogUtil.json]方法进行调试，打印json数据。
 */
object LogUtil {

    // 日志等级常量
    private const val V = 0x01
    private const val D = 0x02
    private const val I = 0x03
    private const val W = 0x04
    private const val E = 0x05
    private const val WTF = 0x06
    private const val JSON = 0x07
    private const val JSON_INDENT = 4 // JSON 缩进
    private var isShowLog = true // 是否显示日志，默认为true.

    /**
     * 初始化方法，用于设置是否显示日志
     */
    @JvmStatic
    fun init(isShowLog: Boolean) {
        this.isShowLog = isShowLog
    }

    /**
     * Debug级别的日志输出方法
     */
    @JvmStatic
    fun d() {
        printLog(D, null, null)
    }

    @JvmStatic
    fun d(msg: Any?) {
        printLog(D, null, msg)
    }

    @JvmStatic
    fun d(tag: String?, msg: Any?) {
        printLog(D, tag, msg)
    }

    /**
     * Error级别的日志输出方法
     */
    @JvmStatic
    fun e() {
        printLog(E, null, null)
    }

    @JvmStatic
    fun e(msg: Any?) {
        printLog(E, null, msg)
    }

    @JvmStatic
    fun e(tag: String?, msg: Any?) {
        printLog(E, tag, msg)
    }

    /**
     * Verbose级别的日志输出方法
     */
    @JvmStatic
    fun v() {
        printLog(V, null, null)
    }

    @JvmStatic
    fun v(msg: Any?) {
        printLog(V, null, msg)
    }

    @JvmStatic
    fun v(tag: String?, msg: String?) {
        printLog(V, tag, msg)
    }

    /**
     * Info级别的日志输出方法
     */
    @JvmStatic
    fun i() {
        printLog(I, null, null)
    }

    @JvmStatic
    fun i(msg: Any?) {
        printLog(I, null, msg)
    }

    @JvmStatic
    fun i(tag: String?, msg: Any?) {
        printLog(I, tag, msg)
    }

    /**
     * Warning级别的日志输出方法
     */
    @JvmStatic
    fun w() {
        printLog(W, null, null)
    }

    @JvmStatic
    fun w(msg: Any?) {
        printLog(W, null, msg)
    }

    @JvmStatic
    fun w(tag: String?, msg: Any?) {
        printLog(W, tag, msg)
    }

    /**
     * Assert级别的日志输出方法
     */
    @JvmStatic
    fun wtf() {
        printLog(WTF, null, null)
    }

    @JvmStatic
    fun wtf(msg: Any?) {
        printLog(WTF, null, msg)
    }

    @JvmStatic
    fun wtf(tag: String?, msg: Any?) {
        printLog(WTF, tag, msg)
    }

    /**
     * 输出格式化的JSON日志
     */
    @JvmStatic
    fun json(jsonStr: String?) {
        printLog(JSON, null, jsonStr, false)
    }

    @JvmStatic
    fun json(jsonStr: String?, isFormatJson: Boolean) {
        printLog(JSON, null, jsonStr, isFormatJson)
    }

    @JvmStatic
    fun json(tag: String?, jsonStr: String?) {
        printLog(JSON, tag, jsonStr)
    }

    @JvmStatic
    fun json(tag: String?, jsonStr: String?, isFormatJson: Boolean) {
        printLog(JSON, tag, jsonStr, isFormatJson)
    }

    /**
     * 打印日志的核心方法。
     *
     * @param type 日志类型 Log type
     * @param tagStr 标签
     * @param objectMsg 日志消息
     * @param isFormatJson 是否格式化 JSON 内容
     */
    private fun printLog(type: Int, tagStr: String?, objectMsg: Any?, isFormatJson: Boolean = false) {
        if (!isShowLog) {
            return
        }
        val stackTrace = Thread.currentThread().stackTrace

        // 寻找调用该方法的堆栈元素. Look for the stack element that called the method
        val callerIndex = stackTrace.indexOfLast { it.className == this::class.java.name } + 1

        // 获取标签，如果为空，则取调用位置的文件名作为标签.
        // Gets the label, if empty, takes the file name of the call location as the label
        val tag = tagStr ?: stackTrace[callerIndex].fileName

        // 格式化堆栈元素，获取类名、方法名和行号
        // Format the stack element to get the class name, method name, and line number
        val logStr = formatStackTraceElement(stackTrace[callerIndex])

        val message = objectMsg?.toString() ?: "Log with null Object"

        if (type != JSON) {
            logMessage(type, tag, "$logStr => $message")
        } else {
            try {
                val jsonContent = if (!isFormatJson) {
                    // 直接返回 JSON 内容. Return JSON content directly
                    message
                } else {
                    // 格式化 JSON 内容. Format the JSON content
                    when {
                        message.startsWith("{") -> {
                            JSONObject(message).toString(JSON_INDENT)
                        }
                        message.startsWith("[") -> {
                            JSONArray(message).toString(JSON_INDENT)
                        }
                        else -> {
                            // 不是 JSON 格式的字符串. Not a JSON formatted string
                            message
                        }
                    }
                }
                printJsonContent(tag, logStr, jsonContent)
            } catch (e: JSONException) {
                logMessage(E, tag, "${e.cause?.message} \n $message")
                return
            }
        }
    }

    /**
     * 打印日志信息。
     *
     * Print log information
     *
     * @param type 日志类型
     * @param tag 标签
     * @param message 日志消息
     */
    private fun logMessage(type: Int, tag: String, message: String) {
        when (type) {
            V -> Log.v(tag, message) // 输出 verbose 级别的日志
            D -> Log.d(tag, message) // 输出 debug 级别的日志
            I -> Log.i(tag, message) // 输出 info 级别的日志
            W -> Log.w(tag, message) // 输出 warning 级别的日志
            E -> Log.e(tag, message) // 输出 error 级别的日志
            WTF -> Log.wtf(tag, message) // 输出 assert 级别的日志
        }
    }

    /**
     * 格式化堆栈元素，返回类名、方法名和行号组成的字符串。
     *
     * Formats the stack element to return a string consisting of the class name, method name, and line number.
     *
     * @param stackTraceElement 堆栈元素
     * @return 类名、方法名和行号组成的字符串
     *         A string of class names, method names, and line numbers
     */
    private fun formatStackTraceElement(stackTraceElement: StackTraceElement): String {
        val className = stackTraceElement.fileName // 获取类名
        val methodName = stackTraceElement.methodName // 获取方法名
        val lineNumber = stackTraceElement.lineNumber // 获取行号

        return "($className:$lineNumber)#${methodName.substring(0, 1).uppercase(Locale.getDefault()) + methodName.substring(1)}"
    }

    /**
     * 打印 JSON 内容。
     *
     * Print the JSON content.
     *
     * @param tag 标签
     * @param logStr 日志信息
     * @param jsonContent JSON 内容
     */
    private fun printJsonContent(tag: String, logStr: String, jsonContent: String) {
        if (jsonContent.isEmpty()) {
            logMessage(E, tag, "Empty or Null json content")
            return
        }

        printLine(tag, true)

        val message = "$logStr${"\n"}$jsonContent"
        val lines = message.split("\n").toTypedArray()

        val formattedJsonContent = kotlin.text.StringBuilder()
        for (line in lines) {
            // 添加表格线格式的消息内容. Add message content formatted as tabular lines
            formattedJsonContent.append("║ ").append(line).append("\n")
        }

        // 如果消息内容超过 3200 字符 If the message content exceeds 3200 characters
        if (formattedJsonContent.length > 3200) {
            logMessage(W, tag, "jsonContent.length = ${formattedJsonContent.length}")
            val chunkCount = formattedJsonContent.length / 3200 // 计算消息内容分块数量. Calculate the number of message content chunks
            var i = 0
            while (i <= chunkCount) {
                val start = 3200 * i
                val end = kotlin.comparisons.minOf(3200 * (i + 1), formattedJsonContent.length)
                logMessage(W, tag, formattedJsonContent.substring(start, end)) // 按块输出消息内容. Output message content in chunks
                i++
            }
        } else {
            logMessage(W, tag, formattedJsonContent.toString())
        }

        printLine(tag, false)
    }

    /**
     * 打印表格线。
     *
     * Print table lines
     *
     * @param tag 标签
     * @param isStart 是否是起始位置. Whether it is the starting position
     */
    private fun printLine(tag: String, isStart: Boolean) {
        val lineSymbol = if (isStart)
            "╔═══════════════════════════════════════════════════════════════════════════════════════"
        else
            "╚═══════════════════════════════════════════════════════════════════════════════════════"
        logMessage(D, tag, lineSymbol)
    }
}